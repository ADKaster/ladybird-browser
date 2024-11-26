/*
 * Copyright (c) 2020, Andreas Kling <andreas@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Error.h>
#include <AK/Vector.h>

namespace IPC {

class Decoder;
class Encoder;
class Message;
class MessageBuffer;
class File;
class Stub;

class TransportSocket;
class TransportMach;

template<typename T>
ErrorOr<void> encode(Encoder&, T const&);

template<typename T>
ErrorOr<T> decode(Decoder&);

using FdSendVector = Vector<int, 1>;
struct [[nodiscard]] ReadResult {
    Vector<u8> bytes;
    Vector<int> fds;
};

}
