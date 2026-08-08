#pragma once

#include <Base.h>
#include <optional>

#include <reversiblehooks/HookConstants.hpp>
#include "OneWayHook.h"
#include "BaseHook.h"

namespace ReversibleHooks {
namespace ReversibleHook {
/*!
 * @brief Basic hook that inserts a `jmp` at the hooked function and redirects it
 */
class BasicHook : public BaseHook {
public:
    BasicHook(
        std::string_view      name,
        void*                 fnAddressOur,
        void*                 fnAddressGTA,
        bool                  reversed                    = true,
        std::optional<size_t> numStackArgumentsToPreserve = std::nullopt,
        bool                  preserveRegisters           = false
    );
    ~BasicHook() override = default;

    void Switch() override;
    void Check() override;
    const char* Symbol() const override { return "S"; }

    auto GetHookGTAAddress() const noexcept { return m_OneWayFromGTA.GetFrom(); }
    auto GetHookOurAddress() const noexcept { return m_OneWayToGTA.GetFrom(); }

protected:
    OneWayHook m_OneWayFromGTA; //!< Redirect calls from gta to our code
    OneWayHook m_OneWayToGTA;   //!< Redirect calls from our code to gta
};
};
};
