/*
 * Copyright (c) 2022, Andreas Kling <andreas@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Format.h>
#include <AK/Traits.h>
#include <AK/Types.h>
#include <AK/Vector.h>

namespace GC {

template<typename T>
class RefImpl {
public:
    RefImpl() = delete;

    RefImpl(T& ptr)
        : m_ptr(&ptr)
    {
    }

    template<typename U>
    RefImpl(U& ptr)
    requires(IsConvertible<U*, T*>)
        : m_ptr(&static_cast<T&>(ptr))
    {
    }

    template<typename U>
    RefImpl(RefImpl<U> const& other)
    requires(IsConvertible<U*, T*>)
        : m_ptr(other.ptr())
    {
    }

    template<typename U>
    RefImpl& operator=(RefImpl<U> const& other)
    requires(IsConvertible<U*, T*>)
    {
        m_ptr = static_cast<T*>(other.ptr());
        return *this;
    }

    RefImpl& operator=(T& other)
    {
        m_ptr = &other;
        return *this;
    }

    template<typename U>
    RefImpl& operator=(U& other)
    requires(IsConvertible<U*, T*>)
    {
        m_ptr = &static_cast<T&>(other);
        return *this;
    }

    RETURNS_NONNULL T* operator->() const { return m_ptr; }

    [[nodiscard]] T& operator*() const { return *m_ptr; }

    RETURNS_NONNULL T* ptr() const { return m_ptr; }

    RETURNS_NONNULL operator T*() const { return m_ptr; }

    operator T&() const { return *m_ptr; }

private:
    T* m_ptr { nullptr };
};

template<typename T>
class Ref : public RefImpl<T> {
public:
    using RefImpl<T>::RefImpl;
    using RefImpl<T>::operator=;
    using RefImpl<T>::operator*;
    using RefImpl<T>::operator->;
    using RefImpl<T>::operator T*;
    using RefImpl<T>::operator T&;
};
template<typename T>
Ref(T&) -> Ref<T>;

template<typename T>
class PtrImpl {
public:
    constexpr PtrImpl() = default;

    PtrImpl(T& ptr)
        : m_ptr(&ptr)
    {
    }

    PtrImpl(T* ptr)
        : m_ptr(ptr)
    {
    }

    template<typename U>
    PtrImpl(PtrImpl<U> const& other)
    requires(IsConvertible<U*, T*>)
        : m_ptr(other.ptr())
    {
    }

    PtrImpl(RefImpl<T> const& other)
        : m_ptr(other.ptr())
    {
    }

    template<typename U>
    PtrImpl(RefImpl<U> const& other)
    requires(IsConvertible<U*, T*>)
        : m_ptr(other.ptr())
    {
    }

    PtrImpl(nullptr_t)
        : m_ptr(nullptr)
    {
    }

    template<typename U>
    PtrImpl& operator=(PtrImpl<U> const& other)
    requires(IsConvertible<U*, T*>)
    {
        m_ptr = static_cast<T*>(other.ptr());
        return *this;
    }

    PtrImpl& operator=(RefImpl<T> const& other)
    {
        m_ptr = other.ptr();
        return *this;
    }

    template<typename U>
    PtrImpl& operator=(RefImpl<U> const& other)
    requires(IsConvertible<U*, T*>)
    {
        m_ptr = static_cast<T*>(other.ptr());
        return *this;
    }

    PtrImpl& operator=(T& other)
    {
        m_ptr = &other;
        return *this;
    }

    template<typename U>
    PtrImpl& operator=(U& other)
    requires(IsConvertible<U*, T*>)
    {
        m_ptr = &static_cast<T&>(other);
        return *this;
    }

    PtrImpl& operator=(T* other)
    {
        m_ptr = other;
        return *this;
    }

    template<typename U>
    PtrImpl& operator=(U* other)
    requires(IsConvertible<U*, T*>)
    {
        m_ptr = static_cast<T*>(other);
        return *this;
    }

    T* operator->() const
    {
        ASSERT(m_ptr);
        return m_ptr;
    }

    [[nodiscard]] T& operator*() const
    {
        ASSERT(m_ptr);
        return *m_ptr;
    }

    T* ptr() const { return m_ptr; }

    explicit operator bool() const { return !!m_ptr; }
    bool operator!() const { return !m_ptr; }

    operator T*() const { return m_ptr; }

private:
    T* m_ptr { nullptr };
};

template<typename T>
class Ptr : public PtrImpl<T> {
public:
    using PtrImpl<T>::PtrImpl;
    using PtrImpl<T>::operator=;
    using PtrImpl<T>::operator*;
    using PtrImpl<T>::operator->;
    using PtrImpl<T>::operator T*;
    using PtrImpl<T>::operator!;
    using PtrImpl<T>::operator bool;
};
template<typename T>
Ptr(T&) -> Ptr<T>;

template<typename T>
requires(!IsSame<T, nullptr_t>)
Ptr(T*) -> Ptr<T>;

template<typename T>
Ptr(Ref<T>) -> Ptr<T>;

// Non-Owning GC::Ptr
template<typename T>
using RawPtr = Ptr<T>;

// Non-Owning Ref
template<typename T>
using RawRef = Ref<T>;

// Member GC::Ptr, for containers
template<typename T>
class MemberPtr : public PtrImpl<T> {
public:
    using PtrImpl<T>::PtrImpl;
    using PtrImpl<T>::operator=;
    using PtrImpl<T>::operator*;
    using PtrImpl<T>::operator->;
    using PtrImpl<T>::operator T*;
    using PtrImpl<T>::operator!;
    using PtrImpl<T>::operator bool;

    MemberPtr(Ptr<T> const& other)
        : PtrImpl<T>(other.ptr())
    {
    }

    MemberPtr& operator=(Ptr<T> const& other)
    {
        this->m_ptr = other.ptr();
        return *this;
    }
};

// Member GC::Ref, for containers
template<typename T>
class MemberRef : public RefImpl<T> {
public:
    using RefImpl<T>::RefImpl;
    using RefImpl<T>::operator=;
    using RefImpl<T>::operator*;
    using RefImpl<T>::operator->;
    using RefImpl<T>::operator T*;
    using RefImpl<T>::operator T&;

    MemberRef(Ref<T> const& other)
        : RefImpl<T>(*other)
    {
    }

    MemberRef& operator=(Ref<T> const& other)
    {
        this->m_ptr = other.ptr();
        return *this;
    }
};

template<typename T, typename U>
inline bool operator==(PtrImpl<T> const& a, PtrImpl<U> const& b)
{
    return a.ptr() == b.ptr();
}

template<typename T, typename U>
inline bool operator==(PtrImpl<T> const& a, RefImpl<U> const& b)
{
    return a.ptr() == b.ptr();
}

template<typename T, typename U>
inline bool operator==(RefImpl<T> const& a, RefImpl<U> const& b)
{
    return a.ptr() == b.ptr();
}

template<typename T, typename U>
inline bool operator==(RefImpl<T> const& a, PtrImpl<U> const& b)
{
    return a.ptr() == b.ptr();
}

template<typename T>
ReadonlySpan<Ptr<T>> to_unowned_span(Vector<MemberPtr<T>> const& vector)
{
    return { reinterpret_cast<Ptr<T> const*>(vector.data()), vector.size() };
}

template<typename T>
ReadonlySpan<Ref<T>> to_unowned_span(Vector<MemberRef<T>> const& vector)
{
    return { reinterpret_cast<Ref<T> const*>(vector.data()), vector.size() };
}

}

namespace AK {

template<typename T>
struct Traits<GC::Ptr<T>> : public DefaultTraits<GC::Ptr<T>> {
    static unsigned hash(GC::Ptr<T> const& value)
    {
        return Traits<T*>::hash(value.ptr());
    }
};

template<typename T>
struct Traits<GC::Ref<T>> : public DefaultTraits<GC::Ref<T>> {
    static unsigned hash(GC::Ref<T> const& value)
    {
        return Traits<T*>::hash(value.ptr());
    }
};

template<typename T>
struct Formatter<GC::Ptr<T>> : Formatter<T const*> {
    ErrorOr<void> format(FormatBuilder& builder, GC::Ptr<T> const& value)
    {
        return Formatter<T const*>::format(builder, value.ptr());
    }
};

template<Formattable T>
struct Formatter<GC::Ref<T>> : Formatter<T> {
    ErrorOr<void> format(FormatBuilder& builder, GC::Ref<T> const& value)
    {
        return Formatter<T>::format(builder, *value);
    }
};

template<typename T>
requires(!HasFormatter<T>)
struct Formatter<GC::Ref<T>> : Formatter<T const*> {
    ErrorOr<void> format(FormatBuilder& builder, GC::Ref<T> const& value)
    {
        return Formatter<T const*>::format(builder, value.ptr());
    }
};

// Vectors of GC::Ptr<T> and GC::Ref<T> are inherently unsafe.
// The memory that holds the pointers is not managed by the GC, and thus liable to
// cause very hard to debug memory safety issues.
//
// Instead, use GC::MemberPtr<T> and GC::MemberRef<T> when the vector is a
// member variable of a GC-managed object, and thus visited via a visit_edges method.
// Otherwise, Use a RootVector or a ConservativeVector for 'free' vectors.
//
template<typename T>
class Vector<GC::Ptr<T>> {
    static_assert(DependentFalse<T>, "Use Vector of GC::MemberPtr<T> or RootVector instead");
};

template<typename T>
class Vector<GC::Ref<T>> {
    static_assert(DependentFalse<T>, "Use Vector of GC::MemberRef<T> or RootVector instead");
};

}
