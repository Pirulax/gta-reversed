#include "StdInc.h"

#include <fstream>

#include "HookCategory.h"
#include "HookCategoryItem.h"

namespace ReversibleHooks {
bool HookCategoryItem::SetState(HookState state, bool ignoreLock) {
    if (!ignoreLock && m_IsStateLocked) {
        return false;
    }
    assert((m_IsReversed || state == HookState::RedirectToGTA) && "Hooks for functions not `reversed` should redirect to gta");
    const auto prev = m_Hook->State();
    if (!m_Hook->State(state)) {
        return false;
    }
    m_PrevState = prev;
    return true;
}

const char* HookCategoryItem::GetTypeSymbolUI() noexcept {
    using enum ReversibleHook::HookType;
    switch (GetType()) {
    case StaticTwoWay:      return "S";
    case Virtual:           return "V";
    case VirtualDestructor: return "VD";
    case VMTRedirect:       return "VR";
    default:                return "U";
    }
}

void HookCategoryItem::PrintToCSV(std::ofstream& of, const HookCategory& cat) const noexcept {
    std::println(
        of,
        "{},{},0x{:08X},{},{},{}",
        cat.Name(),
        m_Hook->Name(),
        LOG_PTR(GetHookAddressGTA()),
        !!m_IsReversed,
        !!m_IsStateLocked,
        EnumToString(m_Hook->Type()).value_or("Unknown")
    );
}
}; // namespace ReversibleHooks
