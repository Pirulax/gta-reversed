#pragma once

#include <string>
#include <filesystem>
#include <memory>

#include "HookSystem.h"
#include <Enums/eScriptCommands.h>
#include "ReversibleHook/BaseHook.h"
#include "ReversibleHook/VirtualDestructorHook.h"
#include "VMTInfo.h"
#include "HooksUtility.hpp"

//
// Helper macros - For help regarding usage see how they're used (`Find all references` and take a look)
// Generally on top of `InjectHooks` you will need to call `RH_ScopedClass` (or `RH_ScopedNamespace`) and `RH_ScopedCategory`
// `RH_ScopedCategory` generally follows the directory layout - Anything in, lets say, the `Entity` directory should be in the `Entity` category.
// The root folder (source/game_sa) can be referred to by `RH_ScopedCategoryGlobal()` - Thus any source files in it should use this category.
//

// Set scoped namespace name (This only works if you only use `ScopedGlobal` macros)
#define RH_ScopedNamespaceName(ns) \
    ReversibleHooks::ScopeName RHCurrentScopeName {ns};

// Use when `name` is a class
#define RH_ScopedClass(cls) \
    using RHCurrentNS = cls; \
    ReversibleHooks::ScopeName RHCurrentScopeName {#cls};

// Use when `name` is a class
#define RH_ScopedNamedClass(cls, name) \
    using RHCurrentNS = cls; \
    ReversibleHooks::ScopeName RHCurrentScopeName {name};

#define RH_ScopedVirtualClass(cls, addrGTAVtbl, nVirtFns_) \
    using RHCurrentNS = cls; \
    ReversibleHooks::ScopeName RHCurrentScopeName {#cls}; \
    const auto pGTAVTbl = ReversibleHooks::Utility::VMTInfo{ (void**)addrGTAVtbl, nVirtFns_ }; \
    const auto pOurVTbl = ReversibleHooks::Utility::VMTInfo::FindByClassName(#cls, nVirtFns_); \

// Use when `name` is a namespace
#define RH_ScopedNamespace(name) \
    namespace RHCurrentNS = name; \
    ReversibleHooks::ScopeName RHCurrentScopeName {#name};

// Supports nested categories separeted by `/`. Eg.: `Entities/Ped`
#define RH_ScopedCategory(name) \
    ReversibleHooks::ScopeCategory  RHCurrentCat{name};

#define RH_RootCategoryName "Root"
#define RH_GlobalCategoryName "Global"
#define RH_ScopedCategoryGlobal() \
    ReversibleHooks::ScopeCategory  RHCurrentCat{ RH_GlobalCategoryName };

// Install a hook living in the current scoped class/namespace
#define RH_ScopedInstall(fn, fnAddr, ...) \
    ReversibleHooks::Install(RHCurrentCat.name + "/" + RHCurrentScopeName.name, #fn, fnAddr, &RHCurrentNS::fn __VA_OPT__(,) __VA_ARGS__)

// Install a hook on a global function
#define RH_ScopedGlobalInstall(fn, fnAddr, ...) \
    ReversibleHooks::Install(RHCurrentCat.name + "/" + RHCurrentScopeName.name, #fn, fnAddr, &fn __VA_OPT__(,) __VA_ARGS__)

// Tip: If a member function is const just add the `const` keyword after the function arg list;
// Eg.: `void(CRect::*)(float*, float*) const` (Notice the const at the end) (See function `CRect::GetCenter`)
#define RH_ScopedOverloadedInstall(fn, suffix, fnAddr, addrCast, ...) \
    ReversibleHooks::Install(RHCurrentCat.name + "/" + RHCurrentScopeName.name, #fn "-" suffix, fnAddr, static_cast<addrCast>(&RHCurrentNS::fn) __VA_OPT__(,) __VA_ARGS__)

#define RH_ScopedGlobalOverloadedInstall(fn, suffix, fnAddr, addrCast, ...) \
    ReversibleHooks::Install(RHCurrentCat.name + "/" + RHCurrentScopeName.name, #fn "-" suffix, fnAddr, static_cast<addrCast>(&fn) __VA_OPT__(,) __VA_ARGS__)

// Install global `fn` as name `fnName`
#define RH_ScopedNamedGlobalInstall(fn, fnName, fnAddr, ...) \
    ReversibleHooks::Install(RHCurrentCat.name + "/" + RHCurrentScopeName.name, fnName, fnAddr, &fn __VA_OPT__(,) __VA_ARGS__)

// Similar to RH_ScopedInstall but you can specify the name explicitly.
#define RH_ScopedNamedInstall(fn, fnName, fnAddr, ...) \
    ReversibleHooks::Install(RHCurrentCat.name + "/" + RHCurrentScopeName.name, fnName, fnAddr, &RHCurrentNS::fn __VA_OPT__(,) __VA_ARGS__)

#define RH_ScopedVMTOverloadedInstall(fn, suffix, fnGTAAddr, addrCast, ...) \
    ReversibleHooks::InstallVirtual(RHCurrentCat.name + "/" + RHCurrentScopeName.name, #fn "-" suffix, pOurVTbl, FunctionToVoidPtr(static_cast<addrCast>(&RHCurrentNS::fn)), pGTAVTbl, (void*)fnGTAAddr __VA_OPT__(,) __VA_ARGS__)

// Install a hook on a virtual function. To use it, `RH_ScopedVirtualClass` must be used instead of `RH_ScopedClass`
#define RH_ScopedNamedVMTInstall(fn, fnName, fnGTAAddr, ...) \
    ReversibleHooks::InstallVirtual(RHCurrentCat.name + "/" + RHCurrentScopeName.name, fnName, pOurVTbl, FunctionToVoidPtr(&RHCurrentNS::fn), pGTAVTbl, (void*)fnGTAAddr __VA_OPT__(,) __VA_ARGS__)

// Install a hook on a virtual function. To use it, `RH_ScopedVirtualClass` must be used instead of `RH_ScopedClass`
#define RH_ScopedVMTInstall(fn, fnGTAAddr, ...) \
    RH_ScopedNamedVMTInstall(fn, #fn, fnGTAAddr __VA_OPT__(,) __VA_ARGS__)

//! Install a script hook
#define RH_ScopedInstallScriptCommand(cmd) \
    ReversibleHooks::InstallScriptCommand(RHCurrentCat.name + "/" + RHCurrentScopeName.name, cmd)

// Install constructor (possibly overloaded)
#define RH_ScopedConstructorInstall(fnAddr, suffix, opts, ...) \
    ReversibleHooks::InstallConstructor<RHCurrentNS __VA_OPT__(,) __VA_ARGS__>(RHCurrentCat.name + "/" + RHCurrentScopeName.name, suffix, fnAddr, opts)

// Install a virtual destructor hook
#define RH_ScopedVMTDestructorInstall(fnGTAAddr, ...) \
    ReversibleHooks::InstallVirtualDestructor<RHCurrentNS>(RHCurrentCat.name + "/" + RHCurrentScopeName.name, pOurVTbl, pGTAVTbl, fnGTAAddr __VA_OPT__(, ) __VA_ARGS__)

// Install classic destructor hook (For virtual ones use RH_ScopedDestructorInstall)
#define RH_ScopedDestructorInstall(fnGTAAddr, ...) \
    ReversibleHooks::Install(RHCurrentCat.name + "/" + RHCurrentScopeName.name, "Desructor", fnGTAAddr, ReversibleHooks::Utility::GetScalarDestructorAddress<RHCurrentNS>() __VA_OPT__(,) __VA_ARGS__)

//#define RH_ScopedVMTAddressChange(fn, fnGTAAddr, ...) \
//    ReversibleHooks::InstallVirtual(RHCurrentCat.name + "/" + RHCurrentScopeName.name, #fn, pGTAVTbl, pOurVTbl, FunctionPointerToVoidP(fnGTAAddr), nVirtFns __VA_OPT__(,) __VA_ARGS__)

namespace ReversibleHooks {
    class RootHookCategory;

    struct ScopeName {
        std::string name{};
    };

    struct ScopeCategory {
        std::string name{};
    };

    struct HookInstallOptions {
        bool reversed{ true };          // Has this function been reversed?
        bool enabled{ reversed };       // Is this hook enabled (eg.: redirects GTA calls to ours or vice versa if disabled) by default?
        bool locked{ !reversed };       // If this hook shouldn't be switchable from the GUI
        int jmpCodeSize{ 5 };
        int stackArguments{ -1 };
    };

    RootHookCategory& GetRootCategory();

    enum class SetCatOrItemStateResult {
        NotFound,
        Locked,
        Done
    };

    SetCatOrItemStateResult SetCategoryOrItemStateByPath(std::string_view path, bool enabled);

    /*!
    * @param category Category's path, eg.: "Global/"
    * @param item     Item to add
    */
    void AddItemToCategory(std::string_view category, std::shared_ptr<ReversibleHook::BaseHook> item);

    namespace detail {
        void HookInstall(std::string_view category, std::string fnName, void* installAddress, void* addressToJumpTo, HookInstallOptions&& opt);
        bool MarkAddressAsHooked(void* address);
    };

    template <typename T>
    static void Install(std::string_view category, std::string fnName, uintptr_t addressGTA, T addressOur, HookInstallOptions opt = {}) {
        detail::HookInstall(category, std::move(fnName), (void*)(addressGTA), FunctionToVoidPtr(addressOur), std::move(opt));
    }

    void InstallVirtual(
        std::string_view   category,
        std::string        fnName,
        Utility::VMTInfo   vmtInfoOur,
        void*              fnAddressOur,
        Utility::VMTInfo   vmtInfoGTA,
        void*              fnAddressGTA,
        HookInstallOptions opt = {}
    );

    template<typename T, typename... Args>
    void InstallConstructor(std::string_view category, std::string_view suffix, uintptr_t addressGTA, bool isVirtual, HookInstallOptions opt = {}) {
        std::string name = "Constructor";
        if (!suffix.empty()) {
            name += "-" + std::string(suffix);
        }
        Install(category, std::move(name), addressGTA, Utility::GetConstructorAddress<T, Args...>(), std::move(opt));
    }

    template <typename T>
    void InstallVirtualDestructor(
        std::string_view   category,
        Utility::VMTInfo   vmtInfoOur,
        Utility::VMTInfo   vmtInfoGTA,
        uintptr_t          addressGTA,
        HookInstallOptions opt = {}
    ) {
        if (!detail::MarkAddressAsHooked((void*)addressGTA)) {
            throw std::runtime_error(std::format("{}/Destructor is hooked to an address ({}) that is already hooked!", category, (void*)(addressGTA)));
        }
        auto hook = std::make_shared<ReversibleHook::VirtualDestructorHook<T>>(
            vmtInfoOur,
            vmtInfoGTA,
            (void*)(addressGTA),
            opt.reversed
        );
        hook->State(opt.enabled);
        hook->LockState(opt.locked);
        AddItemToCategory(category, std::move(hook));
    }

    /*!
    * @brief Hook a script command
    * @param category Category's path, eg.: "Global/"
    * @param cmd     Script command to hook
    */
    void InstallScriptCommand(std::string_view category, eScriptCommands cmd);

    void CheckAll();
    void SwitchHook(std::string_view funcName);

    // Stuff called from InjectHooksMain()

    void OnInjectionBegin(HMODULE hModule);
    void OnInjectionEnd();

    void WriteHooksToFile(const std::filesystem::path&);
};
