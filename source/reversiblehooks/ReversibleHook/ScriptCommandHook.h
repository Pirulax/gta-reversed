#pragma once 

#ifdef NOTSA_WITH_SCRIPT_COMMAND_HOOKS
#include "TwoWayHookBase.h"
#include "eScriptCommands.h"
#include "RunningScript.h"

namespace ReversibleHooks {
namespace ReversibleHook {
/*!
 * @brief 2-way hook for hooking script commands
 */
struct ScriptCommandHook : TwoWayHookBase {
    ScriptCommandHook(eScriptCommands command, bool enabledByDefault = true) :
        TwoWayHookBase{ std::string{::notsa::script::GetScriptCommandName(command)}, TwoWayHookBase::HookType::ScriptCommand },
        m_cmd{command},
        m_originalHandler{CRunningScript::CustomCommandHandlerOf(command)}
    {
        m_IsHooked = true; // Enabled by default
        if (m_IsHooked && !enabledByDefault) {
            Switch(); // Uninstall it
        }
    }

    ~ScriptCommandHook() override {
        State(false);
    }

    void Switch() override {
        using namespace ::notsa::script;

        m_IsHooked = !m_IsHooked;
        CRunningScript::CustomCommandHandlerOf(m_cmd) = m_IsHooked ? m_originalHandler : nullptr;
    }

    void Check() override { /* nop */ }

private:
    eScriptCommands                         m_cmd{};
    ::notsa::script::CommandHandlerFunction m_originalHandler{};
};
};
};
#endif
