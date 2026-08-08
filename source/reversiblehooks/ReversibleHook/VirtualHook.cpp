#include "StdInc.h"

#include "VirtualHook.h"

namespace ReversibleHooks {
namespace ReversibleHook {
VirtualHook::VirtualHook(
    std::string_view name,
    void**           fnVMTEntryOur,
    void**           fnVMTEntryGTA
) :
    VirtualHook{
        std::move(name),
        fnVMTEntryOur,
        *fnVMTEntryOur,
        fnVMTEntryGTA,
        *fnVMTEntryGTA
    }
{
}

VirtualHook::VirtualHook(
    std::string_view name,
    void**           fnVMTEntryOur,
    void*            fnAddressOur,
    void**           fnVMTEntryGTA,
    void*            fnAddressGTA
) :
    TwoWayHookBase{ std::string{ name }, HookType::Virtual },
    m_VirtualDispatchHook{ std::string{ name }, fnVMTEntryOur, fnVMTEntryGTA },
    m_DirectCallHook{ std::string{ name }, fnAddressOur, fnAddressGTA } {
    Switch();
}

void VirtualHook::Switch() {
    m_IsHooked = !m_IsHooked;
    m_DirectCallHook.State(m_IsHooked);
    m_VirtualDispatchHook.State(m_IsHooked);
}
};
};
