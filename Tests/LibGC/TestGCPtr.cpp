/*
 * Copyright (c) 2025, Andrew Kaster <andrew@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGC/Cell.h>
#include <LibGC/Heap.h>
#include <LibGC/Ptr.h>
#include <LibTest/TestSuite.h>

class TestCell : public GC::Cell {
    GC_CELL(TestCell, GC::Cell);
    GC_DECLARE_ALLOCATOR(TestCell);
};

GC_DEFINE_ALLOCATOR(TestCell);

TEST_CASE(test_ref_from_ref)
{
    GC::Heap heap(nullptr, [](auto&) { });

    auto cell = heap.allocate<TestCell>();

    auto ref_from_ref = GC::Ref { *cell };
}
