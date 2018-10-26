//
// SqratOverloadMethods: Overload Methods
//

//
// Copyright (c) 2009 Brandon Jones
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//  claim that you wrote the original software. If you use this software
//  in a product, an acknowledgment in the product documentation would be
//  appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and must not be
//  misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//  distribution.
//

#if !defined(_SQRAT_OVERLOAD_METHODS_H_)
#define _SQRAT_OVERLOAD_METHODS_H_

#ifdef SQMOD_PLUGIN_API
    #include <SqAPI.h>
#else
    #include <squirrel.h>
#endif // SQMOD_PLUGIN_API

#include <sstream>
#include "sqratTypes.h"
#include "sqratUtil.h"
#include "sqratGlobalMethods.h"
#include "sqratMemberMethods.h"

namespace Sqrat {

/// @cond DEV

//
// Overload name generator
//

class SqOverloadName {
public:

    static void Get(const SQChar* name, int args, string & out) {
        SQChar buf[16] = {'_', 'o'};
        itoa(args, &buf[2], 10);
        out.append(buf);
        out.push_back('_');
        out.append(name);
    }

    static string Get(const SQChar* name, int args) {
        string out;
        out.reserve(15);
        Get(name, args, out);
        return out;
    }
};


//
// Don't include the overloaded call forwarder into the templated class
//  to avoid duplicating code that doesn't need to be specialized.
//
inline SQInteger OverloadExecutionForwarder(HSQUIRRELVM vm) {
    // Get the argument count
    const int argCount = static_cast<int>(sq_gettop(vm) - 2);
    // Subtract environment and base name in free variable^
    const SQChar* funcName;
    SQInteger funcNameSize;
    // Get the un-mangled function name (free variable)
    sq_getstringandsize(vm, -1, &funcName, &funcNameSize);
    // Generate the overload mangled name
    string overloadName;
    overloadName.reserve(funcNameSize+5);
    SqOverloadName::Get(funcName, argCount, overloadName);
    // Pop the un-mangled closure name from the stack so we can replace it later
    sq_poptop(vm); // `funcName` becomes invalid after this
    // Push the overload mangled name on the stack
    sq_pushstring(vm, overloadName.c_str(), static_cast<SQInteger>(overloadName.size()));
    // Lookup the proper overload and get it on the stack
#if !defined (SCRAT_NO_ERROR_CHECKING)
    if (SQ_FAILED(sq_get(vm, 1))) {
        sq_pushnull(vm); // Push something to take the place of the free variable
        return sq_throwerror(vm, _SC("wrong number of parameters"));
    }
#else
    sq_get(vm, 1);
#endif
    SQFUNCTION f = nullptr;
    // Get the native closure pointer that we must invoke
    SQRESULT res = sq_getnativeclosurepointer(vm, -1, &f);
    if (SQ_FAILED(res)) return res;
    // Make sure a native closure pointer is available
    if (!f) {
        return sq_throwerror(vm, _SC("unable to acquire the proper overload closure"));
    }
    // Attempt to get the free variable containing the native closure pointer on the stack
    const SQChar *name = sq_getonefreevariable(vm, 0);
    // This is simply a hack to implement a direct call and gain some performance
    // Since both closures expect a free variable we simply replace the free variable
    //  containing closure name with the free variable containing the closure pointer

    // Perform a direct call and store the result
    res = f(vm);
    // If there was a free variable and the closure on the stack was native
    // Then we must push something to take the place of the free variable
    if (name && name[0] == '@') {
        sq_pushnull(vm);
    }
    // Return the result back to the caller
    return res;
}

//
// Squirrel Overload Functions
//

template <class R>
class SqOverload {
public:

    static SQInteger Func(HSQUIRRELVM vm) {
        return OverloadExecutionForwarder(vm);
    }
};


//
// void return specialization
//

template <>
class SqOverload<void> {
public:

    static SQInteger Func(HSQUIRRELVM vm) {
        return OverloadExecutionForwarder(vm);
    }
};

// Global Overloaded Function Resolver
template <class R,class... A> SQFUNCTION SqGlobalOverloadedFunc(R (* /*method*/)(A...)) {
    return SqGlobal<R>::template GetProxy<2, true, A...>();
}

// Global Overloaded Function Resolver
template <class R,class... A> SQFUNCTION SqGlobalOverloadedFunc(R& (* /*method*/)(A...)) {
    return SqGlobal<R&>::template GetProxy<2, true, A...>();
}

// Member Global Overloaded Function Resolver
template <class R,class T,class... A> SQFUNCTION SqMemberGlobalOverloadedFunc(R (* /*method*/)(T, A...)) {
    return SqGlobal<R>::template GetProxy<1, true, T, A...>();
}

// Member Global Overloaded Function Resolver
template <class R,class T,class... A> SQFUNCTION SqMemberGlobalOverloadedFunc(R& (* /*method*/)(T, A...)) {
    return SqGlobal<R&>::template GetProxy<1, true, T, A...>();
}

// Member Overloaded Function Resolver
template <class C, class R,class... A> SQFUNCTION SqMemberOverloadedFunc(R (C::* /*method*/)(A...)) {
    return SqMember<C,R>::template GetProxy<true, A...>();
}

// Member Overloaded Function Resolver
template <class C, class R,class... A> SQFUNCTION SqMemberOverloadedFunc(R (C::* /*method*/)(A...) const) {
    return SqMember<C,R>::template GetProxyC<true, A...>();
}

// Member Overloaded Function Resolver
template <class C, class R,class... A> SQFUNCTION SqMemberOverloadedFunc(R& (C::* /*method*/)(A...)) {
    return SqMember<C,R&>::template GetProxy<true, A...>();
}

// Member Overloaded Function Resolver
template <class C, class R,class... A> SQFUNCTION SqMemberOverloadedFunc(R& (C::* /*method*/)(A...) const) {
    return SqMember<C,R&>::template GetProxyC<true, A...>();
}

//
// Overload handler resolver
//

template <class R>
inline SQFUNCTION SqOverloadFunc(R (* /*method*/)) {
    return &SqOverload<R>::Func;
}

template <class C, class R>
inline SQFUNCTION SqOverloadFunc(R (C::* /*method*/)) {
    return &SqOverload<R>::Func;
}

template <class C, class R>
inline SQFUNCTION SqOverloadFunc(R (C::* /*method*/)() const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1>
inline SQFUNCTION SqOverloadFunc(R (C::* /*method*/)(A1) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2>
inline SQFUNCTION SqOverloadFunc(R (C::* /*method*/)(A1, A2) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3>
inline SQFUNCTION SqOverloadFunc(R (C::* /*method*/)(A1, A2, A3) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4>
inline SQFUNCTION SqOverloadFunc(R (C::* /*method*/)(A1, A2, A3, A4) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5>
inline SQFUNCTION SqOverloadFunc(R (C::* /*method*/)(A1, A2, A3, A4, A5) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline SQFUNCTION SqOverloadFunc(R (C::* /*method*/)(A1, A2, A3, A4, A5, A6) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline SQFUNCTION SqOverloadFunc(R (C::* /*method*/)(A1, A2, A3, A4, A5, A6, A7) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline SQFUNCTION SqOverloadFunc(R (C::* /*method*/)(A1, A2, A3, A4, A5, A6, A7, A8) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline SQFUNCTION SqOverloadFunc(R (C::* /*method*/)(A1, A2, A3, A4, A5, A6, A7, A8, A9) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline SQFUNCTION SqOverloadFunc(R (C::* /*method*/)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline SQFUNCTION SqOverloadFunc(R (C::* /*method*/)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline SQFUNCTION SqOverloadFunc(R (C::* /*method*/)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline SQFUNCTION SqOverloadFunc(R (C::* /*method*/)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13) const ) {
    return &SqOverload<R>::Func;
}

template <class C, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14>
inline SQFUNCTION SqOverloadFunc(R (C::* /*method*/)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14) const ) {
    return &SqOverload<R>::Func;
}

//
// Query argument count
//
template <class R, class... Args>
inline int SqGetArgCount(R (* /*method*/)(Args...)) {
    return sizeof...(Args);
}

//
// Query member function argument count
//
template <class C, class R, class... Args>
inline int SqGetArgCount(R (C::* /*method*/)(Args...)) {
    return sizeof...(Args);
}

//
// Query const member function argument count
//
template <class C, class R, class... Args>
inline int SqGetArgCount(R (C::* /*method*/)(Args...) const) {
    return sizeof...(Args);
}

/// @endcond

}

#endif
