#include "StdInc.h"

#include <extensions/debug.hpp>

#include <reversiblehooks/HooksUtility.hpp>
#include <reversiblehooks/ReversibleHook/VMTRedirectHook.h>

using notsa::debug::GetFunctionInfoAtAddress;


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

            if (*entry->EntryPtr == entry->OriginalFnPtr) {
                throw std::runtime_error(std::format(
                    "VMT redirect Hook (`{}`) for entry {} [{}] was already unhooked from elsewhere",
                    Name(),
                    static_cast<void*>(entry->EntryPtr),
                    GetFunctionInfoAtAddress(std::bit_cast<uintptr_t>(*entry->EntryPtr), true)
                )); 
            }

            *entry->EntryPtr = entry->OriginalFnPtr;
        }
    } else {
        const auto from = state == TwoWayHookState::RedirectToOurs ? &m_FnVMTEntryGTA : &m_FnVMTEntryOur,
                   to   = state == TwoWayHookState::RedirectToOurs ? &m_FnVMTEntryOur : &m_FnVMTEntryGTA;

        Utility::ScopedVirtualProtectModify
            gfrom{ from->EntryPtr, sizeof(from->OriginalFnPtr) },
            gto{ to->EntryPtr, sizeof(to->OriginalFnPtr) };

        if (oldState != TwoWayHookState::Unhooked) {
            if (*from->EntryPtr != *to->EntryPtr || *to->EntryPtr != from->OriginalFnPtr) {
                throw std::runtime_error(std::format(
                    "VMT redirect Hook (`{}`) for {} [{}] (-> {} [{}]) has been modified from elsewhere",
                    Name(),
                    static_cast<void*>(from->EntryPtr),
                    GetFunctionInfoAtAddress(std::bit_cast<uintptr_t>(*from->EntryPtr), true),
                    static_cast<void*>(to->EntryPtr),
                    GetFunctionInfoAtAddress(std::bit_cast<uintptr_t>(*to->EntryPtr), true)
                ));
            }
        }

        *from->EntryPtr = *to->EntryPtr = to->OriginalFnPtr;
    }
}
};
};
