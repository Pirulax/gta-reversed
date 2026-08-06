#pragma once

#include <Base.h>

#include <vector>
#include <string>

#include "BaseHook.h"
#include "SimpleHook.h"
#include "VMTRedirectHook.h"

#include <reversiblehooks/VMTInfo.h>

namespace ReversibleHooks {
namespace ReversibleHook {
/*!
 * @brief Handles hooking of virtual functions, including both direct calls and calls that use the VMT.
 */
struct VirtualDestructor : public Base {
    /*!
     * @brief Constructor
     * @param vmtInfoOur Our VMT's info
     * @param fnAddressOur Address of our function (eg.: &Class::VirtualFunction)
     * @param vmtInfoGTA GTA VMT's info
     * @param fnAddressGTA Address of GTA function (eg.: &Class::VirtualFunction)
     * @param reversed If this hook is reversed (Purely for documentation purposes, doesn't affect the hook's functionality)
     */
    VirtualDestructor(
        VMTInfo vmtInfoOur,
        void*   fnAddressOur,
        VMTInfo vmtInfoGTA,
        void*   fnAddressGTA,
        bool    reversed = true
    );
    ~VirtualDestructor() override = default;

    void        Switch() override;
    void        Check() override { m_DirectCallHook.Check(); m_VirtualDispatchHook.Check(); }
    const char* Symbol() const override { return "V"; }

    auto        GetHookGTAAddress() const { return m_DirectCallHook.GetHookGTAAddress(); }
    auto        GetHookOurAddress() const { return m_DirectCallHook.GetHookOurAddress(); }

private:
    Simple      m_DirectCallHook;      //!< For direct calls (Eg.: Used if virtual call was devirtualized, local objects, )
    VMTRedirect m_VirtualDispatchHook; //!< For virtual destructor calls, these use the VMT and go to a deleting destructor thunk (That takes a bool flag)
};
}; // namespace ReversibleHook
}; // namespace ReversibleHooks
