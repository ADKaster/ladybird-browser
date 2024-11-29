/*
 * Copyright (c) 2024, Andrew Kaster <andrew@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibCore/MachPort.h>
#include <LibTest/TestSuite.h>
#include <fcntl.h>

TEST_CASE(fileport_makeport_badf)
{
    int fd = -1;
    auto fileport_or_error = Core::MachPort::from_fd(fd);
    EXPECT(fileport_or_error.is_error());
    EXPECT_EQ(fileport_or_error.error().code(), EBADF);
}

TEST_CASE(fileport_makeport)
{
    int fd = open("/dev/null", O_RDWR);
    EXPECT(fd > 0);
    auto fileport_or_error = Core::MachPort::from_fd(fd);
    EXPECT(!fileport_or_error.is_error());

    auto ret = ::close(fd);
    EXPECT_EQ(ret, 0);
}

TEST_CASE(fileport_round_trip)
{
    int fd = open("/dev/null", O_RDWR);
    EXPECT(fd > 0);
    auto fileport_or_error = Core::MachPort::from_fd(fd);
    ASSERT(!fileport_or_error.is_error());

    auto ret = ::close(fd);
    EXPECT_EQ(ret, 0);

    auto fileport = fileport_or_error.release_value();
    auto fd_or_error = fileport.release_to_fd();
    EXPECT(!fd_or_error.is_error());

    auto new_fd = fd_or_error.release_value();
    EXPECT(new_fd > 0);

    ret = ::close(new_fd);
    EXPECT_EQ(ret, 0);
}
