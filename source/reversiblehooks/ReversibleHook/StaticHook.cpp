#include "StdInc.h"

#include "StaticHook.h"
#include <reversiblehooks/HooksUtility.hpp>

using namespace ReversibleHooks;

namespace ReversibleHooks {
namespace ReversibleHook {
StaticHook::StaticHook(
    std::string_view name,
    void*            fnAddressOur,
    void*            fnAddressGTA,
    bool             reversed,
    size_t           numStackArgumentsToPreserve,
    bool             preserveRegisters
) :
    TwoWayHookBase{ std::string{ name }, HookType::Static, reversed },
    m_OneWayFromGTA{ std::format("{}_GTA", name), fnAddressGTA, fnAddressOur, numStackArgumentsToPreserve, preserveRegisters },
    m_OneWayToGTA{ std::format("{}_Our", name), fnAddressOur, fnAddressGTA, numStackArgumentsToPreserve, preserveRegisters }
{
    Switch();
}

void StaticHook::Switch() {
    m_IsHooked = !m_IsHooked;

    // Order of hooking/unhooking matters
    if (m_IsHooked) {
        m_OneWayToGTA.State(false);
        m_OneWayFromGTA.State(true);
    } else {
        m_OneWayFromGTA.State(false);
        m_OneWayToGTA.State(true);
    }
}

void StaticHook::Check() {
    if (m_IsHooked) {
        m_OneWayFromGTA.Check();
    } else {
        m_OneWayToGTA.Check();
    }
}

};
};
