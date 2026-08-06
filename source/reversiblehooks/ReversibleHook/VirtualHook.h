#pragma once

#include "Base.h" 

#include <vector>
#include <string>

#include "BaseHook.h"
#include "BasicHook.h"
#include "VMTRedirectHook.h"

#include <reversiblehooks/VMTInfo.h>

namespace ReversibleHooks {
namespace ReversibleHook {
/*!
 * @brief Handles hooking of virtual functions, including both direct calls and calls that use the VMT.
 */
struct VirtualHook : public BaseHook {
    /*!
     * @brief Constructor for hooking virtual functions where the direct call and virtual functions are the same (So basically all virtual functions other than destructors)
     * @param name Name of the function (eg.: `Add` for `CEntity::Add`)
     * @param fnVMTEntryOur Pointer to our VMT's entry of the function (eg.: `&vmt[index]`)
     * @param fnVMTEntryGTA Pointer to GTA's VMT entry of the function (eg.: `&vmt[index]`)
     * @param reversed If this hook is reversed (Purely for documentation purposes, doesn't affect the hook's functionality)
     */
    VirtualHook(
        std::string_view name,
        void**           fnVMTEntryOur,
        void**           fnVMTEntryGTA,
        bool             reversed = true
    );

    /*!
     * @brief Advanced constructor for hooking virtual functions where the direct call and virtual functions aren't the same (For example destructors vs deleting virtual destructor)
     * @param name Name of the function (eg.: `Add` for `CEntity::Add`)
     * @param fnVMTEntryOur Pointer to our VMT's entry of the function (eg.: `&vmt[index]`)
     * @param fnAddressOur Address of our function (eg.: &Class::VirtualFunction)
     * @param fnVMTEntryGTA Pointer to GTA's VMT entry of the function (eg.: `&vmt[index]`)
     * @param fnAddressGTA Address of GTA function (eg.: &Class::VirtualFunction)
     * @param reversed If this hook is reversed (Purely for documentation purposes, doesn't affect the hook's functionality)
     */
    VirtualHook(
        std::string_view name,
        void**           fnVMTEntryOur,
        void*            fnAddressOur,
        void**           fnVMTEntryGTA,
        void*            fnAddressGTA,
        bool             reversed = true
    );
    ~VirtualHook() override = default;

    void        Switch() override;
    void        Check() override { m_DirectCallHook.Check(); m_VirtualDispatchHook.Check(); }
    const char* Symbol() const override { return "V"; }

    auto        GetHookGTAAddress() const { return m_DirectCallHook.GetHookGTAAddress(); }
    auto        GetHookOurAddress() const { return m_DirectCallHook.GetHookOurAddress(); }

private:
    BasicHook      m_DirectCallHook;      //!< For direct calls (Eg.: Explicit calls like `Class::VirtualFunction()`)
    VMTRedirect m_VirtualDispatchHook; //!< For calls that use the VMT (Eg.: `object->VirtualFunction()`)
};
}; // namespace ReversibleHook
}; // namespace ReversibleHooks
