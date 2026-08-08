#include "StdInc.h"
#include <extensions/CommandLine.h>

#include <reversiblehooks/HooksUtility.hpp>
#include "BasicHook.h"

using namespace ReversibleHooks;

namespace ReversibleHooks {
namespace ReversibleHook {
BasicHook::BasicHook(
    std::string_view      name,
    void*                 fnAddressOur,
    void*                 fnAddressGTA,
    bool                  reversed,
    std::optional<size_t> numStackArgumentsToPreserve,
    bool                  preserveRegisters           
) :
    BaseHook{ std::string{ name }, HookType::Basic, reversed },
    m_OneWayFromGTA{ std::format("{}_GTA", name), fnAddressGTA, fnAddressOur, numStackArgumentsToPreserve, preserveRegisters },
    m_OneWayToGTA{ std::format("{}_Our", name), fnAddressOur, fnAddressGTA, numStackArgumentsToPreserve, preserveRegisters }
{
    Switch();
}

void BasicHook::Switch() {
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

void BasicHook::Check() {
    if (m_IsHooked) {
        m_OneWayFromGTA.Check();
    } else {
        m_OneWayToGTA.Check();
    }
}

};
};
