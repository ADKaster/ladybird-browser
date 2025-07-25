/*
 * Copyright (c) 2021, Andreas Kling <andreas@ladybird.org>
 * Copyright (c) 2024, Kenneth Myhra <kennethmyhra@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibGfx/Rect.h>
#include <LibWeb/Bindings/PlatformObject.h>
#include <LibWeb/Bindings/Serializable.h>
#include <LibWeb/Forward.h>

namespace Web::Geometry {

// https://drafts.fxtf.org/geometry/#dictdef-domrectinit
struct DOMRectInit {
    double x { 0.0 };
    double y { 0.0 };
    double width { 0.0 };
    double height { 0.0 };
};

// https://drafts.fxtf.org/geometry/#domrectreadonly
class DOMRectReadOnly
    : public Bindings::PlatformObject
    , public Bindings::Serializable {
    WEB_PLATFORM_OBJECT(DOMRectReadOnly, Bindings::PlatformObject);
    GC_DECLARE_ALLOCATOR(DOMRectReadOnly);

public:
    static WebIDL::ExceptionOr<GC::Ref<DOMRectReadOnly>> construct_impl(JS::Realm&, double x = 0, double y = 0, double width = 0, double height = 0);
    [[nodiscard]] static GC::Ref<DOMRectReadOnly> from_rect(JS::VM&, DOMRectInit const&);
    static GC::Ref<DOMRectReadOnly> create(JS::Realm&);

    virtual ~DOMRectReadOnly() override;

    double x() const { return m_rect.x(); }
    double y() const { return m_rect.y(); }
    double width() const { return m_rect.width(); }
    double height() const { return m_rect.height(); }

    double top() const
    {
        if (isnan(y()) || isnan(height()))
            return NAN;
        return min(y(), y() + height());
    }

    double right() const
    {
        if (isnan(x()) || isnan(width()))
            return NAN;
        return max(x(), x() + width());
    }

    double bottom() const
    {
        if (isnan(y()) || isnan(height()))
            return NAN;
        return max(y(), y() + height());
    }

    double left() const
    {
        if (isnan(x()) || isnan(width()))
            return NAN;
        return min(x(), x() + width());
    }

    virtual HTML::SerializeType serialize_type() const override { return HTML::SerializeType::DOMRectReadOnly; }
    virtual WebIDL::ExceptionOr<void> serialization_steps(HTML::TransferDataEncoder&, bool for_storage, HTML::SerializationMemory&) override;
    virtual WebIDL::ExceptionOr<void> deserialization_steps(HTML::TransferDataDecoder&, HTML::DeserializationMemory&) override;

protected:
    DOMRectReadOnly(JS::Realm&, double x, double y, double width, double height);
    explicit DOMRectReadOnly(JS::Realm&);

    virtual void initialize(JS::Realm&) override;

    Gfx::DoubleRect m_rect;
};

}
