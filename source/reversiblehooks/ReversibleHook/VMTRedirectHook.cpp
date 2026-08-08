#include "StdInc.h"

#include <reversiblehooks/HooksUtility.hpp>
#include <reversiblehooks/ReversibleHook/VMTRedirectHook.h>

namespace ReversibleHooks {
namespace ReversibleHook {
VMTRedirectHook::VMTRedirectHook(
    std::string name,
    void**      fnVMTEntryOur,
    void**      fnVMTEntryGTA
) :
    TwoWayHookBase{ name, HookType::VMTRedirect },
    m_FnVMTEntryOur{ fnVMTEntryOur },
    m_FnVMTEntryGTA{ fnVMTEntryGTA } {
    Switch();
}

void VMTRedirectHook::Switch() {
    m_IsHooked      = !m_IsHooked;

    const auto from = m_IsHooked ? &m_FnVMTEntryGTA : &m_FnVMTEntryOur,
               to   = m_IsHooked ? &m_FnVMTEntryOur : &m_FnVMTEntryGTA;

    Utility::ScopedVirtualProtectModify
        gfrom{ from->EntryPtr, sizeof(from->OriginalFnPtr) },
        gto{ to->EntryPtr, sizeof(to->OriginalFnPtr) };

    *from->EntryPtr = *to->EntryPtr = to->OriginalFnPtr;
}
};
};
