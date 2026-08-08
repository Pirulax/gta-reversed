#pragma once

#include <Base.h>
#include <optional>

#include <reversiblehooks/HookConstants.hpp>
#include "StaticOneWayHook.h"
#include "TwoWayHookBase.h"

namespace ReversibleHooks {
namespace ReversibleHook {
/*!
 * @brief 2-way hook used for static (that is non-virtual) functions
 */
class StaticHook final : public TwoWayHookBase {
public:
    /*!
     * @brief Constructor for hooking static functions
     * @param name Name of the function (eg.: `Add` for `CEntity::Add`)
     * @param fnAddressOur Address of our function (eg.: `&Class::Add`)
     * @param fnAddressGTA Address of GTA's function (eg.: `&Class::Add`)
     * @param numStackArgumentsToPreserve Number of stack arguments to preserve (if any)
     * @param preserveRegisters Whether to preserve registers (if true, must specify `numStackArgumentsToPreserve`)
     */
    StaticHook(
        std::string_view name,
        void*            fnAddressOur,
        void*            fnAddressGTA,
        size_t           numStackArgumentsToPreserve = 0,
        bool             preserveRegisters           = false
    );
    ~StaticHook() override {
        State(false);
    }

    void Switch() override;
    void Check() override;

    auto GetHookGTAAddress() const noexcept { return m_OneWayFromGTA.GetFrom(); }
    auto GetHookOurAddress() const noexcept { return m_OneWayToGTA.GetFrom(); }

protected:
    StaticOneWayHook m_OneWayFromGTA; //!< Redirect calls from gta to our code
    StaticOneWayHook m_OneWayToGTA;   //!< Redirect calls from our code to gta
};
};
};
