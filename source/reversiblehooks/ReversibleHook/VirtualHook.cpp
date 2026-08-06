#include "StdInc.h"

#include "VirtualHook.h"

namespace ReversibleHooks {
namespace ReversibleHook {
Virtual::Virtual(
    std::string_view name,
    void**           fnVMTEntryOur,
    void**           fnVMTEntryGTA,
    bool             reversed
) :
    Virtual{
        std::move(name),
        fnVMTEntryOur,
        *fnVMTEntryOur,
        fnVMTEntryGTA,
        *fnVMTEntryGTA,
        reversed
    }
{
}

Virtual::Virtual(
    std::string_view name,
    void**           fnVMTEntryOur,
    void*            fnAddressOur,
    void**           fnVMTEntryGTA,
    void*            fnAddressGTA,
    bool             reversed
) :
    BaseHook{ std::string{ name }, HookType::Virtual, reversed },
    m_VirtualDispatchHook{ std::string{ name }, fnVMTEntryOur, fnVMTEntryGTA, reversed },
    m_DirectCallHook{ std::string{ name }, (uint32)(fnAddressGTA), fnAddressOur, reversed } {
    Switch();
}

void Virtual::Switch() {
    m_IsHooked = !m_IsHooked;
    m_DirectCallHook.State(m_IsHooked);
    m_VirtualDispatchHook.State(m_IsHooked);
}
};
};
