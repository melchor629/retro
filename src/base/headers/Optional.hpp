#pragma once

#include <memory>
#include <cstdlib>

namespace retro {

    /// Represents nullable objects, but in a safer way.
    /**
     * Optional objects can hold two types or values: a value or nothing. You can check
     * with hasValue() or using the object in a boolean condition if this objects holds
     * a value or nothing. With the operator `->` you can access to the contents of the
     * value (if it has one) and with `*` (or value()) you can get a reference of the object.
     *
     * To use Optional values:
     * ```
     * using namespace retro;
     * using namespace glm;
     * Optional<vec2> value = methodThatReturnsAnOptional();
     * if(value) { std::cout << value->x << ", " << value->y << std::endl; }
     * if(value) { vec2 &vec = *value; vec.x = 1; vec.y = 2; }
     * ...
     * value = {}; //Sets the optional to nothing
     * ```
     **/
    template<typename T>
    class Optional {
        std::shared_ptr<T> obj;

    public:
        constexpr Optional() noexcept {}

        constexpr Optional(std::nullptr_t) noexcept {}

        template<typename U, typename = std::enable_if_t<std::is_convertible<U*, T*>::value>>
        constexpr Optional(const U &value): obj(new U(value)) {}

        template<typename U, typename = std::enable_if_t<std::is_convertible<U*, T*>::value>>
        constexpr Optional(U &&value): obj(new U(std::forward<U>(value))) {}

        Optional(const Optional<T> &other): obj(other.obj) {}

        Optional(Optional<T> &&other): obj(std::move(other.obj)) {}

        Optional& operator=(std::nullptr_t n) {
            obj = n;
            return *this;
        }

        Optional& operator=(const Optional<T> &other) {
            this->obj = other.obj;
            return *this;
        }

        Optional& operator=(Optional<T> &&other) {
            this->obj = std::move(other.obj);
            return *this;
        }

        template<typename U>
        Optional& operator=(const U &value) {
            static_assert(std::is_base_of<T, U>::value, "Cannot do that, type U is not base of T");
            /*if(obj) {
                memcpy((void*) this->obj.get(), (void*) &value, sizeof(T));
            } else*/ {
                this->obj.reset((U*) new U(value));
            }
            return *this;
        }

        constexpr const T* operator->() const { return obj.get(); }
        constexpr T* operator->() { return obj.get(); }
        constexpr const T& operator*() const& { return *obj; }
        constexpr T& operator*() & { return *obj; }

        constexpr explicit operator bool() const noexcept { return !!obj; }
        constexpr bool hasValue() const { return !!obj; }

        constexpr T& value() & { return *obj; }
        constexpr const T& value() const& { return *obj; }

        constexpr void value(T &v) { obj = v; }
        constexpr void value(T &&v) { obj.reset(new T(std::forward<T>(v))); }

    };

    template<typename T>
    constexpr bool operator==(const Optional<T> &l, const Optional<T> &r) {
        return &l == &r;
    }

    template<typename T>
    constexpr bool operator!=(const Optional<T> &l, const Optional<T> &r) {
        return &l != &r;
    }

    template<typename T>
    constexpr bool operator<(const Optional<T> &l, const Optional<T> &r) {
        return &l < &r;
    }

    template<typename T>
    constexpr bool operator<=(const Optional<T> &l, const Optional<T> &r) {
        return &l <= &r;
    }

    template<typename T>
    constexpr bool operator>(const Optional<T> &l, const Optional<T> &r) {
        return &l > &r;
    }

    template<typename T>
    constexpr bool operator>=(const Optional<T> &l, const Optional<T> &r) {
        return &l >= &r;
    }


    template<typename T>
    constexpr bool operator==(const Optional<T> &l, const T &r) {
        return &l == r;
    }

    template<typename T>
    constexpr bool operator==(const T &l, const Optional<T> &r) {
        return l == &r;
    }

    template<typename T>
    constexpr bool operator!=(const Optional<T> &l, const T &r) {
        return &l != r;
    }

    template<typename T>
    constexpr bool operator!=(const T &l, const Optional<T> &r) {
        return l != &r;
    }

    template<typename T>
    constexpr bool operator<(const Optional<T> &l, const T &r) {
        return &l < r;
    }

    template<typename T>
    constexpr bool operator<(const T &l, const Optional<T> &r) {
        return l < &r;
    }

    template<typename T>
    constexpr bool operator<=(const Optional<T> &l, const T &r) {
        return &l <= r;
    }

    template<typename T>
    constexpr bool operator<=(const T &l, const Optional<T> &r) {
        return l <= &r;
    }

    template<typename T>
    constexpr bool operator>(const Optional<T> &l, const T &r) {
        return &l > r;
    }

    template<typename T>
    constexpr bool operator>(const T &l, const Optional<T> &r) {
        return l > &r;
    }

    template<typename T>
    constexpr bool operator>=(const Optional<T> &l, const T &r) {
        return &l == r;
    }

    template<typename T>
    constexpr bool operator>=(const T &l, const Optional<T> &r) {
        return l >= &r;
    }

}
