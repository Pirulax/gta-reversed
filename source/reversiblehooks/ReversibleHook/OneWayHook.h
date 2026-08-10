#pragma once

#include <utility>

#include "Enums/HookMode.h"
#include "Enums/HookType.h"

#include "StatefulHook.h"

namespace ReversibleHooks {
namespace ReversibleHook {
class OneWayHook : public StatefulHook<bool> {
public:
    using StatefulHook::StatefulHook;

    HookMode Mode() const noexcept final { return HookMode::OneWay; }
};
}; // namespace ReversibleHook
}; // namespace ReversibleHooks

