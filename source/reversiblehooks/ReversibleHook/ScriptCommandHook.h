#pragma once 

#include "BaseHook.h"
#include "eScriptCommands.h"
#include "RunningScript.h"

#ifdef NOTSA_WITH_SCRIPT_COMMAND_HOOKS
namespace ReversibleHooks {
namespace ReversibleHook {
struct ScriptCommandHook : BaseHook {
    ScriptCommandHook(eScriptCommands command, bool reversed = true, bool enabledByDefault = true) :
        BaseHook{ std::string{::notsa::script::GetScriptCommandName(command)}, BaseHook::HookType::ScriptCommand, reversed },
        m_cmd{command},
        m_originalHandler{CRunningScript::CustomCommandHandlerOf(command)}
    {
        m_IsHooked = true; // Enabled by default
        if (m_IsHooked && !enabledByDefault) {
            Switch(); // Uninstall it
        }
    }

    ~ScriptCommandHook() override = default;

    void Switch() override {
        using namespace ::notsa::script;

        m_IsHooked = !m_IsHooked;
        CRunningScript::CustomCommandHandlerOf(m_cmd) = m_IsHooked ? m_originalHandler : nullptr;
    }

    void        Check() override { /* nop */ }
    const char* Symbol() const override { return "C"; }
private:
    eScriptCommands                         m_cmd{};
    ::notsa::script::CommandHandlerFunction m_originalHandler{};
};
};
};
#endif
