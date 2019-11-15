//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_STORAGE_PTR_HPP
#define BOOST_JSON_STORAGE_PTR_HPP

#include <boost/json/config.hpp>
#include <boost/json/storage.hpp>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace boost {
namespace json {

/** Manages a type-erased storage object.

    This container is used to hold a shared reference
    to a @ref storage object.
*/
class storage_ptr
{
    template<class T>
    friend class scoped_storage;

    storage* p_ = nullptr;

    inline
    void
    addref() const noexcept
    {
        if(p_ && p_->counted_)
            ++p_->refs_;
    }

    inline
    void
    release() const noexcept
    {
        if( p_ && p_->counted_ &&
            --p_->refs_ == 0)
            delete p_;
    }

    explicit
    storage_ptr(
        storage* p) noexcept
        : p_(p)
    {
        BOOST_JSON_ASSERT(p);
    }

public:
    /** Default constructor.

        This constructs a default storage pointer.
        The default storage is not reference counted,
        uses global operator new and delete to obtain
        memory, and requires calls to `deallocate`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    storage_ptr() = default;

    /** Construct a pointer to default storage.

        This constructs a default storage pointer.
        The default storage is not reference counted,
        uses global operator new and delete to obtain
        memory, and requires calls to `deallocate`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    storage_ptr(
        std::nullptr_t) noexcept
        : storage_ptr()
    {
    }

    /** Destructor.

        This releases the pointed-to storage. If the
        storage is reference counted and this is the
        last reference. the storage object is destroyed.
        If the storage does not require deallocation,
        all memory allocated using this storage is
        invalidated.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    ~storage_ptr()
    {
        release();
    }

    /** Move constructor.

        After construction, the moved-from object
        will point to the default storage.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The storage pointer to construct from.
    */
    storage_ptr(
        storage_ptr&& other) noexcept
        : p_(detail::exchange(
            other.p_, nullptr))
    {
    }

    /** Copy constructor.

        This function acquires shared ownership of
        the storage pointed to by `other`.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The storage pointer to construct from.
    */
    storage_ptr(
        storage_ptr const& other) noexcept
        : p_(other.p_)
    {
        addref();
    }

    /** Move assignment.

        Shared ownership of the storage owned by `*this`
        is released, and shared ownership of the storage
        owned by `other` is acquired by move construction.
        After construction, the moved-from object will
        point to the default storage.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The storage pointer to move.
    */
    storage_ptr&
    operator=(
        storage_ptr&& other) noexcept
    {
        release();
        p_ = detail::exchange(
            other.p_,
            nullptr);
        return *this;
    }

    /** Copy assignment.

        Shared ownership of the storage owned by `*this`
        is released, and shared ownership of the storage
        owned by `other` is acquired by copy construction.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The storage pointer to copy.
    */
    storage_ptr&
    operator=(
        storage_ptr const& other) noexcept
    {
        other.addref();
        release();
        p_ = other.p_;
        return *this;
    }

    /** Return a pointer to the storage object.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    inline
    storage*
    get() const noexcept;

    /** Return a pointer to the storage object.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    storage*
    operator->() const noexcept
    {
        return get();
    }

    /** Return a reference to the storage object.

        @par Precondition

        `this` points to a valid storage object.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    storage&
    operator*() const noexcept
    {
        return *get();
    }

    template<class U, class... Args>
    friend
    storage_ptr
    make_storage(Args&&... args);
};

/** Create a new storage object and return a pointer to it.

    This functions similarly to `make_shared`.

    @par Mandates

    `std::is_base_of_v<storage, U>`

    @par Complexity

    Same as `T(std::forward<Args>(args)...)`.

    @par Exception Safety

    Strong guarantee.

    @param args Parameters forwarded to the constructor of `T`.

    @tparam T the type of the storage object to create.
*/
template<class U, class... Args>
storage_ptr
make_storage(Args&&... args);

/** Return `true` if two storage pointers point to the same object.

    This function returns `true` if the @ref storage
    objects pointed to by `lhs` and `rhs` have the
    same address.
*/
inline
bool
operator==(
    storage_ptr const& lhs,
    storage_ptr const& rhs) noexcept
{
    return lhs.get() == rhs.get();
}

/** Return `true` if two storage pointers point to different objects.

    This function returns `true` if the @ref storage
    objects pointed to by `lhs` and `rhs` have different
    addresses.
*/
inline
bool
operator!=(
    storage_ptr const& lhs,
    storage_ptr const& rhs) noexcept
{
    return lhs.get() != rhs.get();
}

//----------------------------------------------------------

/** A wrapper to provide deterministic lifetime to a @ref storage object.

    This wrapper enables the caller to construct a
    @ref storage objects whose lifetime is controlled
    by the lifetime of the wrapper instead of a
    reference counted.

    @par Example

    This example creates a @ref block_storage with
    bounded lifetime and uses it to parse a JSON,
    then print it to `std::cout`.

    @code

    {
        scoped_storage<block_storage> sp;

        auto jv = parse( str, sp );
    }

    @endcode
*/
template<class T>
class scoped_storage
{
    struct impl : storage
    {
        T t;

        template<class... Args>
        constexpr
        explicit
        impl(Args&&... args)
            : storage(
                T::id(),
                T::need_free(),
                false)
            , t(std::forward<Args>(args)...)
        {
        }

        void*
        allocate(
            std::size_t n,
            std::size_t align) override
        {
            return t.allocate(n, align);
        }

        void
        deallocate(
            void* p,
            std::size_t n,
            std::size_t align) noexcept override
        {
            t.deallocate(p, n, align);
        }
    };

    impl impl_;

public:
    /** Constructor.

        @par Exception Safety

        Any exceptions thrown by `T::T`.

        @param args Arguments forwarded to the
        constructor of the storage object.
    */
    template<class... Args>
    constexpr
    explicit
    scoped_storage(Args&&... args)
        : impl_(std::forward<Args>(args)...)
    {
    }

    /** Return a pointer to the storage object.
    */
    storage*
    get() noexcept
    {
        return &impl_;
    }

    /** Return a pointer to the storage object.
    */
    T*
    operator->() noexcept
    {
        return &impl_.t;
    }

    /** Implicit conversion to @ref storage_ptr.

        This function allows `*this` to be passed
        wherever a @ref storage_ptr is expected.
    */
    operator storage_ptr() noexcept
    {
        return storage_ptr(&impl_);
    }
};

} // json
} // boost

#include <boost/json/impl/storage_ptr.hpp>

#endif
