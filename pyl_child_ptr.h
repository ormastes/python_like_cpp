#pragma once

#include <any>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>
#include <sstream>

namespace pyl {

// Forward declaration of Text (to avoid circular dependency)
class Text;

// -------------------------------------------------------------
// Backtraceable base: child type stores a Parent* back-pointer
// -------------------------------------------------------------
template<typename Parent>
struct Backtraceable {
    Parent* parent = nullptr;  // non-owning

    Parent* get_parent() const noexcept { return parent; }
    bool has_parent() const noexcept    { return parent != nullptr; }
};

// -------------------------------------------------------------
// child_unique_ptr
//
// Features:
// - Unique ownership of T (like std::unique_ptr)
// - Parent* back-pointer for T if it derives Backtraceable<Parent>
// - Optional cycle detection (via Parent::parent chain)
// - Dynamic fields map: ptr["key"] <=> std::any value
// - Dynamic functions:
//     ptr.def<R, Args...>("name", lambda);
//     R r = ptr.call<R>("name", args...);
//     auto res = ptr("name", args...); res.as<R>();
// - Service helpers: to_string, to_full_string, equals, full_equals,
//                    hash, copy, full_copy, length
// Requires C++20 for 'requires' and <concepts>.
// -------------------------------------------------------------
template<
    typename Parent,
    typename T      = Parent,
    typename Deleter = std::default_delete<T>
>
class child_unique_ptr {
public:
    using pointer      = T*;
    using element_type = T;
    using deleter_type = Deleter;

    // -------------------------------
    // call_result: wrapper for std::any
    // -------------------------------
    class call_result {
    public:
        call_result() = default;
        explicit call_result(std::any v)
            : value_(std::move(v)), has_value_(true) {}

        static call_result void_result() { return call_result(); }

        template<typename U>
        U as() const {
            if constexpr (std::is_void_v<U>) {
                // ignore value
                return;
            } else {
                if (!has_value_) {
                    throw std::runtime_error("call_result: no value (void function?)");
                }
                return std::any_cast<U>(value_);
            }
        }

    private:
        std::any value_;
        bool has_value_ = false;
    };

    // -------------------------------
    // constructors / destructor
    // -------------------------------
    constexpr child_unique_ptr() noexcept = default;
    constexpr child_unique_ptr(std::nullptr_t) noexcept {}

    explicit child_unique_ptr(Parent* parent) noexcept
        : parent_(parent) {}

    child_unique_ptr(Parent* parent, pointer p) noexcept
        : parent_(parent) {
        reset(p);
    }

    child_unique_ptr(Parent* parent, pointer p, const Deleter& d) noexcept
        : parent_(parent), deleter_(d) {
        reset(p);
    }

    child_unique_ptr(const child_unique_ptr&)            = delete;
    child_unique_ptr& operator=(const child_unique_ptr&) = delete;

    child_unique_ptr(child_unique_ptr&& other) noexcept
        : parent_(other.parent_),
          ptr_(other.release()),
          deleter_(std::move(other.deleter_)),
          dyn_fields_(std::move(other.dyn_fields_)),
          dyn_fns_(std::move(other.dyn_fns_)) {
        set_parent_on_child();
    }

    child_unique_ptr& operator=(child_unique_ptr&& other) noexcept {
        if (this != &other) {
            pointer p = other.release();
            reset(p);
            deleter_    = std::move(other.deleter_);
            dyn_fields_ = std::move(other.dyn_fields_);
            dyn_fns_    = std::move(other.dyn_fns_);
        }
        return *this;
    }

    ~child_unique_ptr() {
        reset(nullptr);
    }

    // -------------------------------
    // pointer-like API
    // -------------------------------
    pointer get()  const noexcept       { return ptr_; }
    T&      operator*() const           { return *ptr_; }
    pointer operator->() const noexcept { return ptr_; }
    explicit operator bool() const noexcept { return ptr_ != nullptr; }

    deleter_type&       get_deleter()       noexcept { return deleter_; }
    const deleter_type& get_deleter() const noexcept { return deleter_; }

    Parent* parent() const noexcept { return parent_; }

    pointer release() noexcept {
        clear_parent_on_child();
        pointer p = ptr_;
        ptr_ = nullptr;
        // dynamic maps stay attached to the pointer instance
        return p;
    }

    void reset(pointer p = pointer()) noexcept {
        if (p && parent_ && would_create_cycle(p, parent_)) {
            // Silently refuse adoption to prevent cycles
            return;
        }

        if (ptr_) {
            clear_parent_on_child();
            deleter_(ptr_);
        }
        ptr_ = p;
        set_parent_on_child();
        // dynamic data remains attached to this pointer object
    }

    template<typename... Args>
    void emplace(Args&&... args) {
        reset(new T(std::forward<Args>(args)...));
    }

    void swap(child_unique_ptr& other) noexcept {
        using std::swap;
        swap(ptr_, other.ptr_);
        swap(deleter_, other.deleter_);
        swap(dyn_fields_, other.dyn_fields_);
        swap(dyn_fns_, other.dyn_fns_);

        set_parent_on_child();
        other.set_parent_on_child();
    }

    child_unique_ptr& operator=(std::unique_ptr<T>&& up) noexcept {
        pointer p = up.release();
        reset(p);
        return *this;
    }

    // ---------------------------------------------------------
    // Dynamic field map: ptr["key"] <=> any value
    // ---------------------------------------------------------
private:
    using dyn_field_map_t = std::unordered_map<std::string, std::any>;
    std::unique_ptr<dyn_field_map_t> dyn_fields_;

    dyn_field_map_t& ensure_fields() {
        if (!dyn_fields_) {
            dyn_fields_ = std::make_unique<dyn_field_map_t>();
        }
        return *dyn_fields_;
    }

    const dyn_field_map_t* fields_or_null() const noexcept {
        return dyn_fields_.get();
    }

public:
    class field_proxy {
    public:
        field_proxy(child_unique_ptr* owner, std::string key)
            : owner_(owner), key_(std::move(key)) {}

        // assignment: ptr["k"] = value;
        template<typename U>
        field_proxy& operator=(U&& value) {
            if (!owner_ || !owner_->ptr_) {
                throw std::runtime_error("child_unique_ptr::operator[] assign on null pointer");
            }
            auto& m = owner_->ensure_fields();
            m[key_] = std::any(std::forward<U>(value));
            return *this;
        }

        // implicit conversion: U v = ptr["k"];
        template<typename U>
        operator U() const {
            return as<U>();
        }

        // explicit: ptr["k"].as<U>()
        template<typename U>
        U as() const {
            if (!owner_) {
                throw std::runtime_error("field_proxy: no owner");
            }
            const auto* m = owner_->fields_or_null();
            if (!m) {
                throw std::runtime_error("field_proxy: no dynamic fields map");
            }
            auto it = m->find(key_);
            if (it == m->end()) {
                throw std::runtime_error("field_proxy: key not found: " + key_);
            }
            return std::any_cast<U>(it->second);
        }

        bool exists() const {
            if (!owner_) return false;
            const auto* m = owner_->fields_or_null();
            if (!m) return false;
            return m->find(key_) != m->end();
        }

        // call as function: ptr["fn"](args...).as<R>()
        template<typename... Args>
        call_result operator()(Args&&... args) const {
            if (!owner_) {
                throw std::runtime_error("field_proxy: no owner for function call");
            }
            return (*owner_)(key_, std::forward<Args>(args)...);
        }

    private:
        child_unique_ptr* owner_;
        std::string key_;
    };

    field_proxy operator[](std::string key) {
        if (!ptr_) {
            throw std::runtime_error("child_unique_ptr::operator[] on null pointer");
        }
        return field_proxy(this, std::move(key));
    }

    const field_proxy operator[](std::string key) const {
        if (!ptr_) {
            throw std::runtime_error("child_unique_ptr::operator[] const on null pointer");
        }
        return field_proxy(const_cast<child_unique_ptr*>(this), std::move(key));
    }

    // ---------------------------------------------------------
    // Dynamic functions: def / call / operator()
    // ---------------------------------------------------------
private:
    using dyn_fn_t = std::function<std::any(const std::vector<std::any>&)>;
    using dyn_fn_map_t = std::unordered_map<std::string, dyn_fn_t>;
    std::unique_ptr<dyn_fn_map_t> dyn_fns_;

    dyn_fn_map_t& ensure_fns() {
        if (!dyn_fns_) {
            dyn_fns_ = std::make_unique<dyn_fn_map_t>();
        }
        return *dyn_fns_;
    }

    const dyn_fn_map_t* fns_or_null() const noexcept {
        return dyn_fns_.get();
    }

    template<typename R, typename... Args>
    static std::any call_fn_from_anys(
        const std::function<R(Args...)>& fn,
        const std::vector<std::any>& args
    ) {
        if (args.size() != sizeof...(Args)) {
            throw std::runtime_error("dynamic call: argument count mismatch");
        }
        return call_fn_from_anys_impl<R, Args...>(fn, args,
                                                  std::index_sequence_for<Args...>{});
    }

    template<typename R, typename... Args, std::size_t... I>
    static std::any call_fn_from_anys_impl(
        const std::function<R(Args...)>& fn,
        const std::vector<std::any>& args,
        std::index_sequence<I...>
    ) {
        if constexpr (std::is_void_v<R>) {
            fn(std::any_cast<Args>(args[I])...);
            return std::any{};
        } else {
            return std::any(fn(std::any_cast<Args>(args[I])...));
        }
    }

public:
    // Define a dynamic function:
    //   ptr.def<R, Args...>("name", lambda);
    template<typename R, typename... Args, typename F>
    void def(const std::string& name, F&& f) {
        std::function<R(Args...)> fn(std::forward<F>(f));
        auto& m = ensure_fns();
        m[name] = [fn = std::move(fn)](const std::vector<std::any>& a) -> std::any {
            return call_fn_from_anys<R, Args...>(fn, a);
        };
    }

    // Generic call: returns call_result (wraps std::any)
    template<typename... Args>
    call_result operator()(const std::string& name, Args&&... args) const {
        if (!ptr_) {
            throw std::runtime_error("child_unique_ptr::operator(): null pointer");
        }
        const dyn_fn_map_t* m = fns_or_null();
        if (!m) {
            throw std::runtime_error("child_unique_ptr::operator(): no functions defined");
        }
        auto it = m->find(name);
        if (it == m->end()) {
            throw std::runtime_error("child_unique_ptr::operator(): function not found: " + name);
        }

        std::vector<std::any> packed;
        packed.reserve(sizeof...(Args));
        (packed.emplace_back(std::forward<Args>(args)), ...);

        std::any ret = it->second(packed);
        return call_result(std::move(ret));
    }

    // Typed call convenience: like Java-style
    template<typename R, typename... Args>
    R call(const std::string& name, Args&&... args) const {
        return (*this)(name, std::forward<Args>(args)...).template as<R>();
    }

    // ---------------------------------------------------------
    // Service-style helpers
    // ---------------------------------------------------------
    std::string to_string() const {
        if (!ptr_) return "<null>";

        if constexpr (requires(const T& t) {
            { t.to_string() } -> std::convertible_to<std::string>;
        }) {
            return ptr_->to_string();
        } else if constexpr (requires(std::ostream& os, const T& t) {
            { os << t } -> std::same_as<std::ostream&>;
        }) {
            std::ostringstream oss;
            oss << *ptr_;
            return oss.str();
        } else {
            std::ostringstream oss;
            oss << "<" << typeid(T).name()
                << " @" << static_cast<const void*>(ptr_) << ">";
            return oss.str();
        }
    }

    // No hash() call here (hash may fallback to full_string)
    std::string to_full_string() const {
        std::ostringstream oss;
        oss << "[" << typeid(T).name()
            << " value=" << to_string();
        if constexpr (std::is_base_of_v<Backtraceable<Parent>, T>) {
            if (ptr_ && ptr_->parent) {
                oss << " parent@" << static_cast<const void*>(ptr_->parent);
            } else {
                oss << " parent=null";
            }
        }
        oss << "]";
        return oss.str();
    }

    bool equals(const child_unique_ptr& other) const {
        if (ptr_ == nullptr && other.ptr_ == nullptr) return true;
        if (ptr_ == nullptr || other.ptr_ == nullptr) return false;

        if constexpr (requires(const T& a, const T& b) {
            { a.equals(b) } -> std::convertible_to<bool>;
        }) {
            return ptr_->equals(*other.ptr_);
        } else if constexpr (requires(const T& a, const T& b) {
            { a == b } -> std::convertible_to<bool>;
        }) {
            return *ptr_ == *other.ptr_;
        } else {
            return ptr_ == other.ptr_;
        }
    }

    bool full_equals(const child_unique_ptr& other) const {
        // Same T in template; just delegate for now.
        return equals(other);
    }

    // hash():
    //  - if T has hash(), use it
    //  - else if std::hash<T> is valid, use that
    //  - else: hash(to_full_string())
    std::size_t hash() const {
        if (!ptr_) return 0u;

        if constexpr (requires(const T& t) {
            { t.hash() } -> std::convertible_to<std::size_t>;
        }) {
            return ptr_->hash();
        } else if constexpr (requires(const T& t) {
            { std::hash<T>{}(t) } -> std::convertible_to<std::size_t>;
        }) {
            return std::hash<T>{}(*ptr_);
        } else {
            return std::hash<std::string>{}(to_full_string());
        }
    }

    // full_copy() is the "real" deep-copy policy
    child_unique_ptr full_copy() const {
        if (!ptr_) {
            return child_unique_ptr{parent_};
        }

        if constexpr (requires(const T& t) {
            { t.clone() } -> std::same_as<std::unique_ptr<T>>;
        }) {
            auto up = ptr_->clone();
            return child_unique_ptr{parent_, up.release()};
        } else if constexpr (std::is_copy_constructible_v<T>) {
            return child_unique_ptr{parent_, new T(*ptr_)};
        } else {
            throw std::logic_error("child_unique_ptr::full_copy(): T is not cloneable or copy-constructible");
        }
    }

    // copy() just delegates to full_copy()
    child_unique_ptr copy() const {
        return full_copy();
    }

    std::size_t length() const {
        if (!ptr_) return 0u;

        if constexpr (requires(const T& t) {
            { t.size() } -> std::convertible_to<std::size_t>;
        }) {
            return static_cast<std::size_t>(ptr_->size());
        } else if constexpr (requires(const T& t) {
            { t.length() } -> std::convertible_to<std::size_t>;
        }) {
            return static_cast<std::size_t>(ptr_->length());
        } else {
            throw std::logic_error("child_unique_ptr::length(): T has no size()/length()");
        }
    }

    // Text conversion methods (requires pyl_text.h to be included)
    // These are declared here but implemented via free functions in pyl_text.h
    Text to_text() const;
    Text to_full_text() const;

private:
    Parent* parent_   = nullptr;  // non-owning
    pointer ptr_      = nullptr;  // owned
    Deleter deleter_{};

    static bool would_create_cycle(pointer child, Parent* new_parent) noexcept {
        if constexpr (!std::is_base_of_v<Backtraceable<Parent>, T>) {
            return false;
        } else if constexpr (!std::is_base_of_v<Parent, T>) {
            // can't treat T as Parent; skip cycle detection
            return false;
        } else {
            auto* as_parent = static_cast<Parent*>(child);
            for (Parent* cur = new_parent; cur; cur = cur->parent) {
                if (cur == as_parent) {
                    return true;
                }
            }
            return false;
        }
    }

    void set_parent_on_child() noexcept {
        if (!ptr_) return;
        if constexpr (std::is_base_of_v<Backtraceable<Parent>, T>) {
            ptr_->parent = parent_;
        }
    }

    void clear_parent_on_child() noexcept {
        if (!ptr_) return;
        if constexpr (std::is_base_of_v<Backtraceable<Parent>, T>) {
            ptr_->parent = nullptr;
        }
    }
};

// -------------------------------------------------------------
// make_child_unique_ptr helpers
// -------------------------------------------------------------

// Orphan: parent == nullptr, child's parent pointer is nullptr.
template<typename Parent, typename T = Parent, typename... Args>
child_unique_ptr<Parent, T> make_child_unique_ptr(Args&&... args) {
    child_unique_ptr<Parent, T> tmp;  // parent_ == nullptr
    tmp.emplace(std::forward<Args>(args)...);
    return tmp;
}

// Immediate-parent version
template<typename Parent, typename T = Parent, typename... Args>
child_unique_ptr<Parent, T>
make_child_unique_ptr(Parent* parent, Args&&... args) {
    child_unique_ptr<Parent, T> tmp{parent};
    tmp.emplace(std::forward<Args>(args)...);
    return tmp;
}

} // namespace pyl

// -------------------------------------------------------------
// Implementation of Text conversion methods
// -------------------------------------------------------------
// These need to be after Text is defined, so we include it here
#include "pyl_text.h"

namespace pyl {

template<typename Parent, typename T, typename Deleter>
Text child_unique_ptr<Parent, T, Deleter>::to_text() const {
    return Text{to_string()};
}

template<typename Parent, typename T, typename Deleter>
Text child_unique_ptr<Parent, T, Deleter>::to_full_text() const {
    return Text{to_full_string()};
}

} // namespace pyl
