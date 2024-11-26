/*
 * Copyright (c) 2024, Andrew Kaster <andrew@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

import Bootstrap
import Darwin.Mach
import SystemPackage

public let MACH_MSGH_BITS_REMOTE_MASK: Int32 = 0x0000_001F
public let MACH_MSGH_BITS_LOCAL_MASK: Int32 = 0x0000_1F00
public let MACH_MSGH_BITS_VOUCHER_MASK: Int32 = 0x001F_0000
public let MACH_MSGH_BITS_PORTS_MASK: Int32 = MACH_MSGH_BITS_REMOTE_MASK | MACH_MSGH_BITS_LOCAL_MASK | MACH_MSGH_BITS_VOUCHER_MASK

public func machMsghBits(remote: Int32, local: Int32) -> Int32 {
    return (remote | (local << 8))
}

public func machMsghBitsSetPorts(remote: Int32, local: Int32, voucher: Int32) -> Int32 {
    return ((remote & MACH_MSGH_BITS_REMOTE_MASK) | ((local << 8) & MACH_MSGH_BITS_LOCAL_MASK) | ((voucher << 16) & MACH_MSGH_BITS_VOUCHER_MASK))
}

public func machMsghBitsSet(remote: Int32, local: Int32, voucher: Int32, other: Int32) -> Int32 {
    return (machMsghBitsSetPorts(remote: remote, local: local, voucher: voucher) | (other & ~MACH_MSGH_BITS_PORTS_MASK))
}

public func machMsghBitsRemote(bits: Int32) -> Int32 {
    return (bits & MACH_MSGH_BITS_REMOTE_MASK)
}

public func machMsghBitsLocal(bits: Int32) -> Int32 {
    return ((bits & MACH_MSGH_BITS_LOCAL_MASK) >> 8)
}

public func machMsghBitsVoucher(bits: Int32) -> Int32 {
    return ((bits & MACH_MSGH_BITS_VOUCHER_MASK) >> 16)
}

public func machMsghBitsPorts(bits: Int32) -> Int32 {
    return (bits & MACH_MSGH_BITS_PORTS_MASK)
}

public func machMsghBitsOther(bits: Int32) -> Int32 {
    return (bits & ~MACH_MSGH_BITS_PORTS_MASK)
}

public func machMsghBitsHasRemote(bits: Int32) -> Bool {
    return (machMsghBitsRemote(bits: bits) != MACH_MSGH_BITS_ZERO)
}

public func machMsghBitsHasLocal(bits: Int32) -> Bool {
    return (machMsghBitsLocal(bits: bits) != MACH_MSGH_BITS_ZERO)
}

public func machMsghBitsHasVoucher(bits: Int32) -> Bool {
    return (machMsghBitsVoucher(bits: bits) != MACH_MSGH_BITS_ZERO)
}

extension Mach.Port where RightType == Mach.SendRight {
    public init?(to serviceName: Swift.String) {
        var port: mach_port_t = 0
        let res = bootstrap_look_up(Darwin.bootstrap_port, serviceName, &port)
        guard res == KERN_SUCCESS else {
            return nil
        }
        self.init(name: port)
    }
}
