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
    StaticHook(
        std::string_view name,
        void*            fnAddressOur,
        void*            fnAddressGTA,
        bool             reversed                    = true,
        size_t           numStackArgumentsToPreserve = 0,
        bool             preserveRegisters           = false
    );
    ~StaticHook() override {
        State(false);
    }

    void Switch() override;
    void Check() override;
    const char* Symbol() const override { return "S"; }

    auto GetHookGTAAddress() const noexcept { return m_OneWayFromGTA.GetFrom(); }
    auto GetHookOurAddress() const noexcept { return m_OneWayToGTA.GetFrom(); }

protected:
    StaticOneWayHook m_OneWayFromGTA; //!< Redirect calls from gta to our code
    StaticOneWayHook m_OneWayToGTA;   //!< Redirect calls from our code to gta
};
};
};
