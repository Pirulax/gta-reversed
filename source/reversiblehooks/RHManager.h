#pragma once

#include <chrono>
#include <format>
#include <string_view>
#include <optional>
#include <source_location>
#include <string>

#include <Base.h>
#include <extensions/Singleton.hpp>
#include <Enums/eScriptCommands.h>

#include "HooksUtility.hpp"
#include "RootHookCategory.h"
#include "ReversibleHook/TwoWayHook.h"
#include "ReversibleHook/VirtualDestructorHook.h"
#include "ReversibleHook/StaticTwoWayHook.h"

namespace ReversibleHooks {
class RHManager : public notsa::Singleton<RHManager> {
    using HooksCheckClock = std::chrono::steady_clock;

public:
    RHManager() = default;
    ~RHManager() = default;

public:
    auto GetRootCategory() noexcept { return m_RootHookCategory; }

    enum class SetCatOrItemStateResult {
        NotFound,
        Locked,
        Done
    };

    /*!
     * @brief Set the state of a category or item by its path.
     * @param path Path to the category or item (e.g., "Global/Entity/Ped/SetHealth")
     * @param enabled
     * @return 
     */
    SetCatOrItemStateResult SetCategoryOrItemStateByPath(std::string_view path, bool enabled);

    /*!
    * @brief Run `Check` on all hooks
    */
    void CheckAll();

    /*!
    * @brief Write all hooks to a CSV file
    */
    void WriteHooksToFile(const std::filesystem::path&);

public: // Script hooking functions //
    struct HookInstallOptions {
        using HS = ReversibleHook::TwoWayHookState;

        //! Has this function been reversed?
        bool Reversed{ true };

        //! Unhooked by default?
        //! Mostly just a shortcut for `State` to be set to `HS::Unhooked` by default
        //! (Unless `Reversed` is false, in which case it will be set to `HS::RedirectToGTA`)
        bool Unhooked{ false };

        //! Initial state of the hook
        HS State{ Reversed ? Unhooked ? HS::Unhooked : HS::RedirectToOurs : HS::RedirectToGTA };

        //! If this hook shouldn't be switchable from the GUI
        bool Locked{ !Reversed };

        //! [Virtual Only]
        //! 
        //! Does hook overrides the virtual method of the base class?
        //! 
        //! Must be correctly specified because classes not overriding
        //! will (in our code) inherit the address of the base class's method
        //! causing the same function being hooked from different places
        //! In GTA each class gets it's own function, regardless 
        //! of whether it overrides the base class's method or not.
        bool Overrides{ true };

        //! Number of stack arguments to preserve
        std::optional<size_t> StackArgumentsToPreserve{};

        //! If enabled registers will be saved accross the call, but it requires `StackArgumentsToPreserve` to be set as well
        bool PreserveRegisters{ false };

        //! Where this hook was installed from (used for debugging)
        std::source_location InstallSrcLoc{ std::source_location::current() };          
    };

    /*!
     * @brief Adds a hook to a category
     * @param category Category path (Eg.: "Global/Entity/Ped")
     * @param opt Hook installation options
     */
    void AddHookToCategory(
        std::string_view category,
        HookInstallOptions opt,
        std::shared_ptr<ReversibleHook::TwoWayHook> hook
    );

    /*!
     * @brief Installs a hook for a static function.
     * @tparam T Function type
     * @param category Category path (Eg.: "Global/Entity/Ped")
     * @param fnName Name of the function (Eg.: "SetHealth")
     * @param addressGTA Address of function in GTA code
     * @param addressOur Address of function in our code
     * @param opt Hook installation options
     */
    template <typename T>
    void InstallStatic(
        std::string_view   category,
        std::string        fnName,
        uintptr_t          addressGTA,
        T                  fnOur,
        HookInstallOptions opt = {}
    ) {
        // If `PreserveRegisters` then `StackArgumentsToPreserve` may be 0, we just want to enforce it to be set to prevent bugs
        if (opt.PreserveRegisters && !opt.StackArgumentsToPreserve.has_value()) {
            throw std::runtime_error(std::format("{}/{}: `PreserveRegisters` requires `StackArgumentsToPreserve` to be set!", category, fnName));
        }
        AddHookToCategory(category, opt, std::make_shared<ReversibleHook::StaticTwoWayHook>(
            std::move(fnName),
            Utility::FunctionToVoidPtr(fnOur),
            std::bit_cast<void*>(addressGTA),
            opt.StackArgumentsToPreserve.value_or(0),
            opt.PreserveRegisters
        ));
    }

    /*!
     * @brief Installs a hook for a virtual function of a class T.
     * @param category Category path (Eg.: "Global/Entity/Ped")
     * @param fnName Name of the function (Eg.: "SetHealth")
     * @param vmtInfoOur Class's VMT info on our side
     * @param fnAddressOur Address of our function to call
     * @param vmtInfoGTA Class's VMT info on GTA side
     * @param fnAddressGTA Address of the GTA function to hook
     * @param opt Hook installation options
     */
    void InstallVirtual(
        std::string_view   category,
        std::string        fnName,
        Utility::VMTInfo   vmtInfoOur,
        void*              fnAddressOur,
        Utility::VMTInfo   vmtInfoGTA,
        void*              fnAddressGTA,
        HookInstallOptions opt = {}
    );

    /*!
     * @brief Installs a hook for a constructor of a class T.
     * @tparam T The class
     * @tparam ...Args Constructor arguments (If such constructor doesn't exist the compiler will complain)
     * @param category Category path
     * @param suffix Suffix to append to the hook name
     * @param addressGTA Address of the constructor in GTA
     * @param opt Hook installation options
     */
    template<typename T, typename... Args>
    void InstallConstructor(
        std::string_view   category,
        std::string_view   suffix,
        uintptr_t          addressGTA,
        HookInstallOptions opt = {}
    ) {
        std::string name = "Constructor";
        if (!suffix.empty()) {
            name += "-" + std::string(suffix);
        }
        InstallStatic(
            category,
            std::move(name),
            addressGTA,
            Utility::GetConstructorAddress<T, Args...>(),
            std::move(opt)
        );
    }

    /*!
     * @brief Installs a hook for a virtual destructor of a class T.
     * @tparam T The class
     * @param category Category path
     * @param vmtInfoOur class's VMT in our code
     * @param vmtInfoGTA class's VMT in GTA
     * @param addressGTA Address of the virtual (scalar) destructor in GTA
     * @param opt Hook installation options
     */
    template <typename T>
    void InstallVirtualDestructor(
        std::string_view   category,
        Utility::VMTInfo   vmtInfoOur,
        Utility::VMTInfo   vmtInfoGTA,
        uintptr_t          addressGTA,
        HookInstallOptions opt = {}
    ) {
        AddHookToCategory(category, category, std::make_shared<ReversibleHook::VirtualDestructorHook<T>>(
            vmtInfoOur,
            vmtInfoGTA,
            (void*)(addressGTA)
        ));
    }

    /*!
     * @brief Hook a script command
     * @param category Category's path, eg.: "Global/"
     * @param cmd     Script command to hook
     */
    void InstallScriptCommand(
        std::string_view   category,
        eScriptCommands    cmd,
        HookInstallOptions opt = {}
    );

private:
    std::shared_ptr<ReversibleHooks::RootHookCategory> m_RootHookCategory{ std::make_shared<ReversibleHooks::RootHookCategory>() };
    HooksCheckClock::time_point                        m_LastHooksCheckTime{ HooksCheckClock::now() };
};
}; // namespace ReversibleHooks
