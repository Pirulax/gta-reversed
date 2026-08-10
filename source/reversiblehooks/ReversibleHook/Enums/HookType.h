#pragma once

namespace ReversibleHooks {
namespace ReversibleHook {
/*!
 * @brief Types of hooks available (Based on class type)
 */
enum class HookType {
    StaticOneWay,
    StaticTwoWay,
    Virtual,
    VirtualDestructor,
    ScriptCommand,
    VMTRedirect,
};
}; // namespace ReversibleHook
}; // namespace ReversibleHooks


inline std::optional<const char*> EnumToString(ReversibleHooks::ReversibleHook::HookType t) {
    using enum ReversibleHooks::ReversibleHook::HookType;
    switch (t) {
    case StaticOneWay:  return "StaticOneWay";
    case StaticTwoWay:  return "StaticTwoWay";
    case Virtual:       return "Virtual";
    case ScriptCommand: return "ScriptCommand";
    case VMTRedirect:   return "VMTRedirect";
    default:            return std::nullopt;
    }
}

