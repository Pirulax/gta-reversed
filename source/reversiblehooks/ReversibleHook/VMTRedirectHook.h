#pragma once

#include "Base.h"

#include <vector>
#include <string>

#include <reversiblehooks/ReversibleHook/TwoWayHookBase.h>
#include <reversiblehooks/ReversibleHook/StaticHook.h>

namespace ReversibleHooks {
namespace ReversibleHook {
/*!
 * @brief Redirects calls to a function in the VMT to our own function, and vice versa.
 * @note Only modifies the VMT entries, doesn't handle direct calls to the function (eg.: `CEntity::Add`).
 */
struct VMTRedirectHook final : public TwoWayHookBase {
public:
    VMTRedirectHook(
        std::string name,
        void**      fnVMTEntryOur,
        void**      fnVMTEntryGTA
    );

    ~VMTRedirectHook() override {
        State(false);
    }

    void        Switch() override;
    void        Check() override { /* nop */ }
    auto        GetHookGTAAddress() const noexcept { return m_FnVMTEntryGTA.OriginalFnPtr; }
    auto        GetHookOurAddress() const noexcept { return m_FnVMTEntryOur.OriginalFnPtr; }

private:
    struct VMTEntryInfo {
        void** EntryPtr;                   //!< Pointer to the function's VMT entry (eg.: &vmt[index])
        void*  OriginalFnPtr{ *EntryPtr }; //!< Original function pointer
    } m_FnVMTEntryOur{}, m_FnVMTEntryGTA{};
};
};
};
