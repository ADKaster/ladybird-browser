/*
 * Copyright (c) 2024, Andrew Kaster <andrew@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

import AK
import Darwin.Mach
import Foundation
@_exported import IPCCxx
import SystemPackage

@_extern(c, "fileport_makeport")
func fileport_makeport(_ fd: Int32, _ port: UnsafeMutablePointer<mach_port_t>) -> kern_return_t

@_extern(c, "fileport_makefd")
func fileport_makefd(_ port: mach_port_name_t) -> Int32

public struct TransportMach: ~Copyable {
    private var sendRight: Mach.Port<Mach.SendRight>
    private var receiveRight: Mach.Port<Mach.ReceiveRight>

    public init(name: mach_port_name_t) {
        self.sendRight = .init(name: name)
        self.receiveRight = .init()
    }

    public init?(to serviceName: Swift.String) {
        guard let port = Mach.Port<Mach.SendRight>(to: serviceName) else {
            return nil
        }
        self.sendRight = port
        self.receiveRight = .init()
    }

    static public let IPC_MESSAGE_SEND_ID: mach_msg_id_t = 0x4950_4353  // IPCS

    public func send(bytes: AK.ReadonlyBytes, fds: consuming IPC.FdSendVector) throws {

        // Calculate how much space we need for the message
        // header + body + ool descriptor + N * fds
        let messageSize = mach_msg_size_t(MemoryLayout<mach_msg_base_t>.stride + MemoryLayout<mach_msg_ool_descriptor_t>.stride + fds.count * MemoryLayout<mach_msg_port_descriptor_t>.stride)

        var header = mach_msg_header_t()
        header.msgh_bits = mach_msg_bits_t(machMsghBits(remote: MACH_MSG_TYPE_COPY_SEND, local: MACH_MSGH_BITS_ZERO)) | MACH_MSGH_BITS_COMPLEX
        header.msgh_size = mach_msg_size_t(messageSize)
        header.msgh_remote_port = sendRight.withBorrowedName { name in return name }
        header.msgh_local_port = mach_port_name_t(MACH_PORT_NULL)
        header.msgh_id = TransportMach.IPC_MESSAGE_SEND_ID

        var body = mach_msg_body_t()
        body.msgh_descriptor_count = mach_msg_size_t(1 + fds.count)

        var ool_descriptor = mach_msg_ool_descriptor_t()
        ool_descriptor.address = UnsafeMutableRawPointer(mutating: bytes.data())
        ool_descriptor.size = mach_msg_size_t(bytes.count)
        ool_descriptor.deallocate = 0
        ool_descriptor.copy = mach_msg_copy_options_t(MACH_MSG_VIRTUAL_COPY)
        ool_descriptor.type = mach_msg_descriptor_type_t(MACH_MSG_OOL_DESCRIPTOR)

        let null_port = mach_port_name_t(MACH_PORT_NULL)
        let timeout: mach_msg_timeout_t = 100  // 100ms

        try withUnsafeTemporaryAllocation(byteCount: Int(messageSize), alignment: 4) { buffer throws(NSError) in
            var offset = 0
            buffer.storeBytes(of: header, toByteOffset: 0, as: mach_msg_header_t.self)
            offset += MemoryLayout<mach_msg_header_t>.stride
            buffer.storeBytes(of: body, toByteOffset: offset, as: mach_msg_body_t.self)
            offset += MemoryLayout<mach_msg_body_t>.stride
            buffer.storeBytes(of: ool_descriptor, toByteOffset: offset, as: mach_msg_ool_descriptor_t.self)
            offset += MemoryLayout<mach_msg_ool_descriptor_t>.stride

            for fd in fds {
                var port_descriptor = mach_msg_port_descriptor_t()
                let ret = fileport_makeport(fd, &port_descriptor.name)
                guard ret == KERN_SUCCESS else {
                    // FIXME: Write error, shut down the connection
                    throw NSError(domain: NSMachErrorDomain, code: Int(ret), userInfo: nil)
                }
                port_descriptor.disposition = mach_msg_type_name_t(MACH_MSG_TYPE_MOVE_SEND)
                port_descriptor.type = mach_msg_descriptor_type_t(MACH_MSG_PORT_DESCRIPTOR)
                buffer.storeBytes(of: port_descriptor, toByteOffset: offset, as: mach_msg_port_descriptor_t.self)
                offset += MemoryLayout<mach_msg_port_descriptor_t>.stride
            }

            precondition(offset == messageSize)

            let result = mach_msg(buffer.assumingMemoryBound(to: mach_msg_header_t.self).baseAddress, MACH_SEND_MSG | MACH_SEND_TIMEOUT, messageSize, 0, null_port, timeout, null_port)
            guard result == MACH_MSG_SUCCESS else {
                throw NSError(domain: NSMachErrorDomain, code: Int(result), userInfo: nil)
            }
        }
    }
}
