#include "StdInc.h"

#include <fstream>

#include "HookCategory.h"
#include "HookCategoryItem.h"

namespace ReversibleHooks {
void* HookCategoryItem::GetHookAddressGTA() const noexcept {
    using namespace ReversibleHook;
    switch (m_Hook->Type()) {
    case TwoWayHookBase::HookType::Virtual:
        return std::static_pointer_cast<VirtualHook>(m_Hook)->GetHookGTAAddress();
    case TwoWayHookBase::HookType::Static:
        return std::static_pointer_cast<StaticHook>(m_Hook)->GetHookGTAAddress();
    default:
        return nullptr;
    }
}

void* HookCategoryItem::GetHookAddressOur() const noexcept {
    using namespace ReversibleHook;
    switch (m_Hook->Type()) {
    case TwoWayHookBase::HookType::Virtual:
        return std::static_pointer_cast<VirtualHook>(m_Hook)->GetHookOurAddress();
    case TwoWayHookBase::HookType::Static:
        return std::static_pointer_cast<StaticHook>(m_Hook)->GetHookOurAddress();
    default:
        return nullptr;
    }
}

const char* HookCategoryItem::GetSymbol() const noexcept {
    switch (m_Hook->Type()) {
    case ReversibleHook::TwoWayHookBase::HookType::Virtual:
        return "V";
    case ReversibleHook::TwoWayHookBase::HookType::Static:
        return "S";
    case ReversibleHook::TwoWayHookBase::HookType::ScriptCommand:
        return "SC";
    default:
        return nullptr;
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
