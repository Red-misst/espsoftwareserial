/*
Delegate.h - An efficient interchangeable C function ptr and C++ std::function delegate
Copyright (c) 2019 Dirk O. Kaar. All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __Delegate_h
#define __Delegate_h

#if defined(ESP8266)
#include <c_types.h>
#elif defined(ESP32)
#include <esp_attr.h>
#else
#define ICACHE_RAM_ATTR
#define IRAM_ATTR
#endif

#include <functional>
#include <cstddef>

namespace detail
{

    template<typename A, typename R, typename... P>
    class DelegatePImpl {
    public:
        using target_type = R(P...);
        using FunPtr = R(*)(A, P...);
        using FunctionType = std::function<target_type>;

        DelegatePImpl()
        {
            fn = nullptr;
            new (&obj) A;
        }

        DelegatePImpl(std::nullptr_t)
        {
            fn = nullptr;
            new (&obj) A;
        }

        ~DelegatePImpl()
        {
            if (FUNC == kind)
                functional.~FunctionType();
            else
                obj.~A();
        }

        DelegatePImpl(const DelegatePImpl& del)
        {
            kind = del.kind;
            if (FUNC == del.kind)
            {
                new (&functional) FunctionType(del.functional);
            }
            else
            {
                fn = del.fn;
                new (&obj) A(del.obj);
            }
        }

        DelegatePImpl(DelegatePImpl&& del)
        {
            kind = del.kind;
            if (FUNC == del.kind)
            {
                new (&functional) FunctionType(std::move(del.functional));
            }
            else
            {
                fn = del.fn;
                new (&obj) A(std::move(del.obj));
            }
        }

        DelegatePImpl(FunPtr fn, const A& obj)
        {
            kind = FP;
            DelegatePImpl::fn = fn;
            new (&this->obj) A(obj);
        }

        DelegatePImpl(FunPtr fn, A&& obj)
        {
            kind = FP;
            DelegatePImpl::fn = fn;
            new (&this->obj) A(std::move(obj));
        }

        template<typename F> DelegatePImpl(const F& functional)
        {
            kind = FUNC;
            new (&this->functional) FunctionType(functional);
        }

        template<typename F> DelegatePImpl(F&& functional)
        {
            kind = FUNC;
            new (&this->functional) FunctionType(std::move(functional));
        }

        DelegatePImpl& operator=(const DelegatePImpl& del)
        {
            if (this == &del) return *this;
            if (FUNC == kind && FUNC != del.kind)
            {
                functional.~FunctionType();
                new (&obj) A;
            }
            else if (FUNC != kind && FUNC == del.kind)
            {
                obj.~A();
                new (&this->functional) FunctionType();
            }
            kind = del.kind;
            if (FUNC == del.kind)
            {
                functional = del.functional;
            }
            else
            {
                fn = del.fn;
                obj = del.obj;
            }
            return *this;
        }

        DelegatePImpl& operator=(DelegatePImpl&& del)
        {
            if (this == &del) return *this;
            if (FUNC == kind && FUNC != del.kind)
            {
                functional.~FunctionType();
                new (&obj) A;
            }
            else if (FUNC != kind && FUNC == del.kind)
            {
                obj.~A();
                new (&this->functional) FunctionType();
            }
            kind = del.kind;
            if (FUNC == del.kind)
            {
                functional = std::move(del.functional);
            }
            else
            {
                fn = del.fn;
                obj = std::move(del.obj);
            }
            return *this;
        }

        template<typename F> DelegatePImpl& operator=(const F& functional)
        {
            if (FUNC != kind)
            {
                obj.~A();
                new (&this->functional) FunctionType();
                kind = FUNC;
            }
            this->functional = functional;
            return *this;
        }

        template<typename F> DelegatePImpl& operator=(F&& functional)
        {
            if (FUNC != kind)
            {
                obj.~A();
                new (&this->functional) FunctionType();
                kind = FUNC;
            }
            this->functional = std::move(functional);
            return *this;
        }

        DelegatePImpl& IRAM_ATTR operator=(std::nullptr_t)
        {
            if (FUNC == kind)
            {
                functional.~FunctionType();
                new (&obj) A;
            }
            kind = FP;
            fn = nullptr;
            obj = {};
            return *this;
        }

        operator bool() const
        {
            if (FUNC == kind)
            {
                return functional ? true : false;
            }
            else
            {
                return fn;
            }
        }

        R IRAM_ATTR operator()(P... args) const
        {
            if (FUNC == kind)
            {
                return functional(std::forward<P...>(args...));
            }
            else
            {
                return fn(obj, std::forward<P...>(args...));
            }
        }

    protected:
        enum { FUNC, FP } kind = FP;
        union {
            FunctionType functional;
            struct {
                FunPtr fn;
                A obj;
            };
        };
    };

    template<typename R, typename... P>
    class DelegatePImpl<void, R, P...> : public std::function<R(P...)> {
    public:
        using std::function<R(P...)>::function;
        using std::function<R(P...)>::operator=;
    };

    template<typename A, typename R>
    class DelegateImpl {
    public:
        using target_type = R();
        using FunPtr = R(*)(A);
        using FunctionType = std::function<target_type>;

        DelegateImpl()
        {
            fn = nullptr;
            new (&obj) A;
        }

        DelegateImpl(std::nullptr_t)
        {
            fn = nullptr;
            new (&obj) A;
        }

        ~DelegateImpl()
        {
            if (FUNC == kind)
                functional.~FunctionType();
            else
                obj.~A();
        }

        DelegateImpl(const DelegateImpl& del)
        {
            kind = del.kind;
            if (FUNC == del.kind)
            {
                new (&functional) FunctionType(del.functional);
            }
            else
            {
                fn = del.fn;
                new (&obj) A(del.obj);
            }
        }

        DelegateImpl(DelegateImpl&& del)
        {
            kind = del.kind;
            if (FUNC == del.kind)
            {
                new (&functional) FunctionType(std::move(del.functional));
            }
            else
            {
                fn = del.fn;
                new (&obj) A(std::move(del.obj));
            }
        }

        DelegateImpl(FunPtr fn, const A& obj)
        {
            kind = FP;
            DelegateImpl::fn = fn;
            new (&this->obj) A(obj);
        }

        DelegateImpl(FunPtr fn, A&& obj)
        {
            kind = FP;
            DelegateImpl::fn = fn;
            new (&this->obj) A(std::move(obj));
        }

        template<typename F> DelegateImpl(const F& functional)
        {
            kind = FUNC;
            new (&this->functional) FunctionType(functional);
        }

        template<typename F> DelegateImpl(F&& functional)
        {
            kind = FUNC;
            new (&this->functional) FunctionType(std::move(functional));
        }

        DelegateImpl& operator=(const DelegateImpl& del)
        {
            if (this == &del) return *this;
            if (FUNC == kind && FUNC != del.kind)
            {
                functional.~FunctionType();
                new (&obj) A;
            }
            else if (FUNC != kind && FUNC == del.kind)
            {
                obj.~A();
                new (&this->functional) FunctionType();
            }
            kind = del.kind;
            if (FUNC == del.kind)
            {
                functional = del.functional;
            }
            else
            {
                fn = del.fn;
                obj = del.obj;
            }
            return *this;
        }

        DelegateImpl& operator=(DelegateImpl&& del)
        {
            if (this == &del) return *this;
            if (FUNC == kind && FUNC != del.kind)
            {
                functional.~FunctionType();
                new (&obj) A;
            }
            else if (FUNC != kind && FUNC == del.kind)
            {
                obj.~A();
                new (&this->functional) FunctionType();
            }
            kind = del.kind;
            if (FUNC == del.kind)
            {
                functional = std::move(del.functional);
            }
            else
            {
                fn = del.fn;
                obj = std::move(del.obj);
            }
            return *this;
        }

        template<typename F> DelegateImpl& operator=(const F& functional)
        {
            if (FUNC != kind)
            {
                obj.~A();
                new (&this->functional) FunctionType();
                kind = FUNC;
            }
            this->functional = functional;
            return *this;
        }

        template<typename F> DelegateImpl& operator=(F&& functional)
        {
            if (FUNC != kind)
            {
                obj.~A();
                new (&this->functional) FunctionType();
                kind = FUNC;
            }
            this->functional = std::move(functional);
            return *this;
        }

        DelegateImpl& IRAM_ATTR operator=(std::nullptr_t)
        {
            if (FUNC == kind)
            {
                functional.~FunctionType();
                new (&obj) A;
            }
            kind = FP;
            fn = nullptr;
            obj = {};
            return *this;
        }

        operator bool() const
        {
            if (FUNC == kind)
            {
                return functional ? true : false;
            }
            else
            {
                return fn;
            }
        }

        R IRAM_ATTR operator()() const
        {
            if (FUNC == kind)
            {
                return functional();
            }
            else
            {
                return fn(obj);
            }
        }

    protected:
        enum { FUNC, FP } kind = FP;
        union {
            FunctionType functional;
            struct {
                FunPtr fn;
                A obj;
            };
        };
    };

    template<typename R>
    class DelegateImpl<void, R> {
    public:
        using target_type = R();
        using FunPtr = R(*)();
        using FunctionType = std::function<target_type>;

        DelegateImpl()
        {
            fn = nullptr;
        }

        ~DelegateImpl()
        {
            if (FUNC == kind)
                functional.~FunctionType();
        }

        DelegateImpl(const DelegateImpl& del)
        {
            kind = del.kind;
            if (FUNC == del.kind)
            {
                new (&functional) FunctionType(del.functional);
            }
            else
            {
                fn = del.fn;
            }
        }

        DelegateImpl(DelegateImpl&& del)
        {
            kind = del.kind;
            if (FUNC == del.kind)
            {
                new (&functional) FunctionType(std::move(del.functional));
            }
            else
            {
                fn = del.fn;
            }
        }

        DelegateImpl(FunPtr fn)
        {
            kind = FP;
            DelegateImpl::fn = fn;
        }

        template<typename F> DelegateImpl(const F& functional)
        {
            kind = FUNC;
            new (&this->functional) FunctionType(functional);
        }

        template<typename F> DelegateImpl(F&& functional)
        {
            kind = FUNC;
            new (&this->functional) FunctionType(std::move(functional));
        }

        DelegateImpl& operator=(const DelegateImpl& del)
        {
            if (this == &del) return *this;
            if (FUNC == kind && FUNC != del.kind)
            {
                functional.~FunctionType();
            }
            else if (FUNC != kind && FUNC == del.kind)
            {
                new (&this->functional) FunctionType();
            }
            kind = del.kind;
            if (FUNC == del.kind)
            {
                functional = del.functional;
            }
            else
            {
                fn = del.fn;
            }
            return *this;
        }

        DelegateImpl& operator=(DelegateImpl&& del)
        {
            if (this == &del) return *this;
            if (FUNC == kind && FUNC != del.kind)
            {
                functional.~FunctionType();
            }
            else if (FUNC != kind && FUNC == del.kind)
            {
                new (&this->functional) FunctionType();
            }
            kind = del.kind;
            if (FUNC == del.kind)
            {
                functional = std::move(del.functional);
            }
            else
            {
                fn = del.fn;
            }
            return *this;
        }

        template<typename F> DelegateImpl& operator=(const F& functional)
        {
            if (FUNC != kind)
            {
                new (&this->functional) FunctionType();
                kind = FUNC;
            }
            this->functional = functional;
            return *this;
        }

        template<typename F> DelegateImpl& operator=(F&& functional)
        {
            if (FUNC != kind)
            {
                new (&this->functional) FunctionType();
                kind = FUNC;
            }
            this->functional = std::move(functional);
            return *this;
        }

        DelegateImpl& IRAM_ATTR operator=(std::nullptr_t)
        {
            if (FUNC == kind)
            {
                functional.~FunctionType();
            }
            kind = FP;
            fn = nullptr;
            return *this;
        }

        operator bool() const
        {
            if (FUNC == kind)
            {
                return functional ? true : false;
            }
            else
            {
                return fn;
            }
        }

        R IRAM_ATTR operator()() const
        {
            if (FUNC == kind)
            {
                return functional();
            }
            else
            {
                return fn();
            }
        }

    protected:
        enum { FUNC, FP } kind = FP;
        union {
            FunctionType functional;
            FunPtr fn;
        };
    };

    template<typename R = void, typename A = void, typename... P>
    class Delegate : public detail::DelegatePImpl<A, R, P...>
    {
        using detail::DelegatePImpl<A, R, P...>::DelegatePImpl;
    };

    template<typename R, typename A>
    class Delegate<R, A> : public detail::DelegateImpl<A, R>
    {
        using detail::DelegateImpl<A, R>::DelegateImpl;
    };

}

template<typename R, typename A = void, typename... P> class Delegate;
template<typename R, typename A, typename... P> class Delegate<R(P...), A> : public detail::Delegate<R, A, P...>
{
    using detail::Delegate<R, A, P...>::Delegate;
};
template<typename R, typename... P> class Delegate<R(P...)> : public detail::Delegate<R, void, P...>
{
    using detail::Delegate<R, void, P...>::Delegate;
};

#endif // __Delegate_h