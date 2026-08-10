#pragma once

namespace ReversibleHooks {
namespace ReversibleHook {
enum class TwoWayHookState {
    Unhooked,
    RedirectToGTA,
    RedirectToOurs,
};
}; // namespace ReversibleHook
}; // namespace ReversibleHooks

std::optional<const char*> EnumToString(ReversibleHooks::ReversibleHook::TwoWayHookState state) {
    using State = ReversibleHooks::ReversibleHook::TwoWayHookState;
    switch (state) {
    case State::Unhooked:       return "Unhooked";
    case State::RedirectToGTA:  return "RedirectToGTA";
    case State::RedirectToOurs: return "RedirectToOurs";
    default:                    return nullptr;
    }
}
