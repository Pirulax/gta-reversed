#pragma once

#include "Enums/HookMode.h"
#include "Enums/TwoWayHookState.h"

#include "StatefulHook.h"

namespace ReversibleHooks {
namespace ReversibleHook {
struct TwoWayHook : public StatefulHook<TwoWayHookState> {
    using StatefulHook::StatefulHook;

    HookMode Mode() const noexcept final { return HookMode::TwoWay; }

    virtual void* GetHookAddressGTA() const noexcept = 0;
    virtual void* GetHookAddressOur() const noexcept = 0;
};
}; // namespace ReversibleHook
}; // namespace ReversibleHooks
