#pragma once

#include <Base.h>

#include <utility>

#include "Enums/HookMode.h"
#include "Enums/HookType.h"

#include "Hook.h"

namespace ReversibleHooks {
namespace ReversibleHook {
/*!
 * @brief Base class for hooks that maintain a state
 * @note This class is not meant to be used directly, it's mostly here to reduce code duplication. Can't be a part of `Hook` due to the template parameter
 * @tparam StateType Type of the state
 */
template<typename StateType>
class StatefulHook : public Hook {
public:
    using Hook::Hook;

    bool State(StateType state) {
        if (m_State == state) {
            return false; // No change
        }
        try {
            ApplyNewState(state, m_State);
        } catch (const std::exception& e) {
            NOTSA_UNREACHABLE("Failed to apply new state for hook {}: {}", m_Name, e.what()); // We're exiting here because i'm lazy to properly restore state after an exception
        }
        m_State = state;
        return true;
    }

    auto State() const noexcept { return m_State; }

protected:
    /*!
     * @brief Apply state to the hook
     * @note `m_State` needn't to be updated here, it will be updated by `State()`
     * @param state The new state to apply
     */
    virtual void ApplyNewState(StateType state, StateType oldState) = 0;

protected:
    StateType m_State{}; //!< Current state of the hook
};
}; // namespace ReversibleHook
}; // namespace ReversibleHooks

