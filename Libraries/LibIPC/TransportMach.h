/*
 * Copyright (c) 2024, Andrew Kaster <andrew@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Platform.h>
#include <AK/Queue.h>

#if !defined(AK_OS_MACH)
#    error "TransportMach is only available on Mach platforms"
#endif

#include <AK/Function.h>
#include <AK/Vector.h>
#include <LibCore/MachPort.h>
#include <sys/event.h>

namespace IPC {

class TransportMach {
    AK_MAKE_NONCOPYABLE(TransportMach);
    AK_MAKE_DEFAULT_MOVABLE(TransportMach);

public:
    struct WithoutSendPortTag { };

    explicit TransportMach(Core::MachPort&& send_port);
    explicit TransportMach(WithoutSendPortTag);
    ~TransportMach();

    void set_up_read_hook(Function<void()> hook);

    bool is_open() const;
    void close();

    void wait_until_readable();

    ErrorOr<void> transfer(ReadonlyBytes bytes_to_write, Vector<int, 1> const& unowned_fds);

    struct [[nodiscard]] ReadResult {
        Vector<u8> bytes;
        Vector<int> fds;
    };
    ReadResult read_as_much_as_possible_without_blocking(Function<void()> schedule_shutdown);

    // Obnoxious name to make it clear that this is a dangerous operation.
    ErrorOr<mach_port_name_t> release_underlying_transport_for_transfer();
    ErrorOr<Core::MachPort> clone_for_transfer();

private:
    void set_up_kqueue();
    void send_handshake();
    void handle_incoming_handshake();

    Optional<Core::MachPort> m_send_port;
    Core::MachPort m_receive_port;
    RefPtr<Core::Notifier> m_notifier;
    Function<void()> m_read_hook;
    Queue<ByteBuffer, 32> m_send_queue;
    int m_kqueue { -1 };
};

}
