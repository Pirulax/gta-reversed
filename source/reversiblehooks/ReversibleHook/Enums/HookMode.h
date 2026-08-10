#pragma once

namespace ReversibleHooks {
namespace ReversibleHook {
/*!
 * @brief Modes of hooks available (TwoWay or OneWay)
 */
enum class HookMode {
    TwoWay, //!< Can go either way (GTA->Ours or Ours->GTA)
    OneWay, //!< Can only go one way (GTA->Ours or Ours->GTA)
};
}; // namespace ReversibleHook
}; // namespace ReversibleHooks
