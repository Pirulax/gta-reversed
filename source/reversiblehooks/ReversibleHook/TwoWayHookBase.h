#pragma once

#include <string>

namespace ReversibleHooks {
namespace ReversibleHook {
struct TwoWayHookBase {
    enum class HookType { // Sadly can't use `Type` alone as it's some function..
        OneWay,
        Static,
        Virtual,
        ScriptCommand,
        VMTRedirect,
    };

    enum class HookState {
        REDIRECT_TO_GTA,
        REDIRECT_TO_OURS,
        UNHOOKED
    };

    TwoWayHookBase(std::string name, HookType type) :
        m_Name{ std::move(name) },
        m_Type{ type }
    {
    }

    virtual ~TwoWayHookBase() = default;
    virtual void Switch() = 0;
    virtual void Check() = 0;

    /*!
    * @brief Hook/unhook
    * @param hooked If this hook should be installed/uninstalled (true/false)
    * @returns If the state has changed
    */
    bool State(bool hooked) {
        if (hooked == m_IsHooked) {
            return false; // No change
        }
        Switch();
        return true;
    }

    const auto& Name()     const { return m_Name; }
    const auto  Type()     const { return m_Type; }
    const auto  Hooked()   const { return m_IsHooked; }

protected:
    bool        m_IsHooked{};   // Is hook installed (true => yes, gta calls are redirected to our code, false => our code is redirected to the gta function)
    std::string m_Name{};       // Name of function, eg.: `Add` (Referring to CEntity::Add)
    HookType    m_Type{};
};
};
};

inline std::optional<const char*> EnumToString(ReversibleHooks::ReversibleHook::TwoWayHookBase::HookType t) {
    using enum ReversibleHooks::ReversibleHook::TwoWayHookBase::HookType;
    switch (t) {
    case OneWay:        return "OneWay";
    case Static:        return "Static";
    case Virtual:       return "Virtual";
    case ScriptCommand: return "ScriptCommand";
    case VMTRedirect:   return "VMTRedirect";
    default:            return std::nullopt;
    }
}
