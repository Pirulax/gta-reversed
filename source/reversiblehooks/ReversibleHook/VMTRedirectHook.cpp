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
    TwoWayHook{ std::move(name) },
    m_FnVMTEntryOur{ fnVMTEntryOur },
    m_FnVMTEntryGTA{ fnVMTEntryGTA }
{
}

void VMTRedirectHook::ApplyNewState(TwoWayHookState state, TwoWayHookState oldState) {
    if (state == TwoWayHookState::Unhooked) {
        for (auto* const entry : { &m_FnVMTEntryOur, &m_FnVMTEntryGTA }) {
            Utility::ScopedVirtualProtectModify g{ entry->EntryPtr, sizeof(entry->OriginalFnPtr) };
            *entry->EntryPtr = entry->OriginalFnPtr;
        }
    } else {
        const auto from = state == TwoWayHookState::RedirectToOurs ? &m_FnVMTEntryGTA : &m_FnVMTEntryOur,
                   to   = state == TwoWayHookState::RedirectToOurs ? &m_FnVMTEntryOur : &m_FnVMTEntryGTA;

        Utility::ScopedVirtualProtectModify
            gfrom{ from->EntryPtr, sizeof(from->OriginalFnPtr) },
            gto{ to->EntryPtr, sizeof(to->OriginalFnPtr) };

        if (*from->EntryPtr != *to->EntryPtr) {

        }

        *from->EntryPtr = *to->EntryPtr = to->OriginalFnPtr;
    }
}
};
};
