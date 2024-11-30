/*
 * Copyright (c) 2024, Andrew Kaster <andrew@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Format.h>
#include <LibCore/Notifier.h>
#include <LibCore/Platform/MachMessageTypes.h>
#include <LibCore/System.h>
#include <LibIPC/TransportMach.h>

namespace IPC {
;

TransportMach::TransportMach(Core::MachPort&& send_port)
    : m_send_port(move(send_port))
    , m_receive_port(MUST(Core::MachPort::create_with_right(Core::MachPort::PortRight::Receive)))
{
    VERIFY(m_send_port->right() == Core::MachPort::PortRight::Send);

    set_up_kqueue();
    send_handshake();
}

TransportMach::TransportMach(WithoutSendPortTag)
    : m_send_port(OptionalNone {})
    , m_receive_port(MUST(Core::MachPort::create_with_right(Core::MachPort::PortRight::Receive)))
{
    set_up_kqueue();
}

TransportMach::~TransportMach()
{
    close();
}

void TransportMach::set_up_kqueue()
{
    // FIXME: Can we use one kqueue for all mach ports in the process?
    m_kqueue = kqueue();
    if (m_kqueue < 0) {
        dbgln("kqueue: {}", strerror(errno));
        VERIFY_NOT_REACHED();
    }
    auto port = m_receive_port.port();

    kevent64_s event {};
    EV_SET64(&event, port, EVFILT_MACHPORT, EV_ADD, 0, 0, 0, 0, 0, 0);

    auto rc = kevent64(m_kqueue, &event, 1, nullptr, 0, 0, nullptr);
    if (rc < 0) {
        dbgln("kevent: {}", strerror(errno));
        VERIFY_NOT_REACHED();
    }

    m_notifier = Core::Notifier::construct(m_kqueue, Core::Notifier::Type::Read);
    m_notifier->on_activation = [this] {
        kevent64_s evt {};
        auto const ret = kevent64(m_kqueue, nullptr, 0, &evt, 1, 0, nullptr);
        if (ret < 0) {
            dbgln("kevent: {}", strerror(errno));
            m_notifier->set_enabled(false);
            return;
        }
        VERIFY(evt.filter == EVFILT_MACHPORT);
        VERIFY(evt.ident == m_receive_port.port());

        if (!m_send_port.has_value()) {
            handle_incoming_handshake();
            return;
        }

        if (m_read_hook)
            m_read_hook();
    };
}

void TransportMach::send_handshake()
{
    VERIFY(m_send_port.has_value());

    // Send our own task port to the server so they can query statistics about us
    Core::Platform::MessageWithSelfTaskPort message {};
    message.header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSGH_BITS_ZERO) | MACH_MSGH_BITS_COMPLEX;
    message.header.msgh_size = sizeof(message);
    message.header.msgh_remote_port = m_send_port->port();
    message.header.msgh_local_port = MACH_PORT_NULL;
    message.header.msgh_id = Core::Platform::SELF_TASK_PORT_MESSAGE_ID;
    message.body.msgh_descriptor_count = 1;
    message.port_descriptor.name = m_receive_port.port();
    message.port_descriptor.disposition = MACH_MSG_TYPE_COPY_SEND;
    message.port_descriptor.type = MACH_MSG_PORT_DESCRIPTOR;

    mach_msg_timeout_t const timeout = 100; // milliseconds

    auto const send_result = mach_msg(&message.header, MACH_SEND_MSG | MACH_SEND_TIMEOUT, message.header.msgh_size, 0, MACH_PORT_NULL, timeout, MACH_PORT_NULL);
    if (send_result != KERN_SUCCESS) {
        dbgln("Failed to send message to server: {}", mach_error_string(send_result));
        VERIFY_NOT_REACHED();
    }
}

void TransportMach::handle_incoming_handshake()
{
    Core::Platform::ReceivedMachMessage message {};
    // FIXME: Grab info from a trailer and use it to verify that we always get a message from the correct sender
    constexpr mach_msg_options_t options = MACH_RCV_MSG;

    auto const rc = mach_msg(&message.header, options, 0, sizeof(message), m_receive_port.port(), MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
    VERIFY(rc == KERN_SUCCESS);

    if (message.header.msgh_id != Core::Platform::SELF_TASK_PORT_MESSAGE_ID) {
        dbgln("Unexpected message ID: {}", message.header.msgh_id);
        VERIFY_NOT_REACHED();
    }

    auto const& body = message.body.parent;

    VERIFY(body.body.msgh_descriptor_count == 1);
    VERIFY(MACH_MSGH_BITS_LOCAL(message.header.msgh_bits) == MACH_MSG_TYPE_MOVE_SEND);
    VERIFY(body.port_descriptor.disposition == MACH_MSG_TYPE_COPY_SEND);
    VERIFY(body.port_descriptor.type == MACH_MSG_PORT_DESCRIPTOR);

    m_send_port = Core::MachPort::adopt_right(body.port_descriptor.name, Core::MachPort::PortRight::Send);
}

void TransportMach::set_up_read_hook(Function<void()> hook)
{
    m_read_hook = move(hook);
}

bool TransportMach::is_open() const
{
    return m_kqueue >= 0 && m_send_port.has_value();
}

void TransportMach::close()
{
    if (m_kqueue >= 0) {
        m_notifier->set_enabled(false);
        kevent64_s event {};
        EV_SET64(&event, m_receive_port.port(), EVFILT_MACHPORT, EV_DELETE, 0, 0, 0, 0, 0, 0);
        kevent64(m_kqueue, &event, 1, nullptr, 0, 0, nullptr);
        ::close(m_kqueue);
        m_kqueue = -1;
    }
}

void TransportMach::wait_until_readable()
{
    pollfd the_fd = { .fd = m_kqueue, .events = POLLIN, .revents = 0 };
    int const timeout = -1;

    ErrorOr<int> result { 0 };
    do {
        result = Core::System::poll({ &the_fd, 1 }, timeout);
    } while (result.is_error() && result.error().code() == EINTR);

    VERIFY(!result.is_error());
    VERIFY(the_fd.revents & POLLIN == POLLIN);
}

ErrorOr<void> TransportMach::transfer(ReadonlyBytes bytes_to_write, Vector<int, 1> const& unowned_fds)
{

    return {};
}

TransportMach::ReadResult TransportMach::read_as_much_as_possible_without_blocking(Function<void()> schedule_shutdown)
{
    return {};
}

}
