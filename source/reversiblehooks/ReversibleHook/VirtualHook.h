#pragma once

#include "Base.h" 

#include <vector>
#include <string>

#include "TwoWayHookBase.h"
#include "StaticHook.h"
#include "VMTRedirectHook.h"

#include <reversiblehooks/VMTInfo.h>

namespace ReversibleHooks {
namespace ReversibleHook {
/*!
 * @brief Handles hooking of virtual functions, including both direct calls and calls that use the VMT.
 */
struct VirtualHook : public TwoWayHookBase {
    /*!
     * @brief Constructor for hooking virtual functions where the direct call and virtual functions are the same (So basically all virtual functions other than destructors)
     * @param name Name of the function (eg.: `Add` for `CEntity::Add`)
     * @param fnVMTEntryOur Pointer to our VMT's entry of the function (eg.: `&vmt[index]`)
     * @param fnVMTEntryGTA Pointer to GTA's VMT entry of the function (eg.: `&vmt[index]`)
     */
    VirtualHook(
        std::string_view name,
        void**           fnVMTEntryOur,
        void**           fnVMTEntryGTA
    );

    /*!
     * @brief Advanced constructor for hooking virtual functions where the direct call and virtual functions aren't the same (For example destructors vs deleting virtual destructor)
     * @param name Name of the function (eg.: `Add` for `CEntity::Add`)
     * @param fnVMTEntryOur Pointer to our VMT's entry of the function (eg.: `&vmt[index]`)
     * @param fnAddressOur Address of our function (eg.: &Class::VirtualFunction)
     * @param fnVMTEntryGTA Pointer to GTA's VMT entry of the function (eg.: `&vmt[index]`)
     * @param fnAddressGTA Address of GTA function (eg.: &Class::VirtualFunction)
     */
    VirtualHook(
        std::string_view name,
        void**           fnVMTEntryOur,
        void*            fnAddressOur,
        void**           fnVMTEntryGTA,
        void*            fnAddressGTA
    );

    ~VirtualHook() override {
        State(false);
    }

    void Switch() override;
    void Check() override { m_DirectCallHook.Check(); m_VirtualDispatchHook.Check(); }
    auto GetHookGTAAddress() const { return m_DirectCallHook.GetHookGTAAddress(); }
    auto GetHookOurAddress() const { return m_DirectCallHook.GetHookOurAddress(); }

private:
    StaticHook      m_DirectCallHook;      //!< For direct calls (Eg.: Explicit calls like `Class::VirtualFunction()`)
    VMTRedirectHook m_VirtualDispatchHook; //!< For calls that use the VMT (Eg.: `object->VirtualFunction()`)
};
}; // namespace ReversibleHook
}; // namespace ReversibleHooks
