#include "StdInc.h"

#include "CommandLine.h"

#include <span>
#include <Utility.h>
#include <app_debug.h>

namespace CommandLine {
bool                          s_UnhookAll{ false };
std::vector<std::string_view> s_UnhookSome{};
std::vector<std::string_view> s_UnhookExcept{};

/* Path to save the hooks dump file to */
fs::path s_DumpHooksPath{};

/* Wait for debugger before continuing */
bool s_WaitForDebugger{ false };

/* exe path */
fs::path s_ExePath{};

void ProcessArgument(std::string_view arg) {
    if (arg == "--debug") {
        s_WaitForDebugger = true;
    } else if (arg == "--unhook-all") {
        if (!s_UnhookExcept.empty()) {
            NOTSA_LOG_WARN("--unhook-except has been called previously, unhook-all will be effective.");
            s_UnhookExcept.clear(); // we aren't gonna use it.
        }
        if (!s_UnhookSome.empty()) {
            NOTSA_LOG_WARN("--unhook-some has been called previously, unhook-all will be effective.");
            s_UnhookSome.clear(); // we aren't gonna use it.
        }
        s_UnhookAll = true;
    } else if (arg.starts_with("--unhook-except=")) {
        if (s_UnhookAll) {
            NOTSA_LOG_WARN("--unhook-all has been called previously, unhook-all will be effective.");
        } else {
            if (!s_UnhookSome.empty()) {
                NOTSA_LOG_WARN("--unhook has been called previously, unhook-except will be effective.");
                s_UnhookSome.clear(); // we aren't gonna use it.
            }

            for (auto hook : SplitStringView(arg.substr(arg.find('=') + 1), ",")) {
                s_UnhookExcept.emplace_back(std::move(hook));
            }
        }
    } else if (arg.starts_with("--unhook=")) {
        if (s_UnhookAll) {
            NOTSA_LOG_WARN("--unhook-all has been called previously, unhook-all will be effective.");
        } else if (!s_UnhookExcept.empty()) {
            NOTSA_LOG_WARN("--unhook-except has been called previously, unhook-except will be effective.");
        } else {
            for (auto hook : SplitStringView(arg.substr(arg.find('=') + 1), ",")) {
                s_UnhookSome.emplace_back(std::move(hook));
            }
        }
    } else if (arg.starts_with("--dump-hooks-to=")) {
        if (const auto eqPos = arg.find('='); eqPos != std::string_view::npos) {
            s_DumpHooksPath = arg.substr(eqPos + 1);
        } else {
            NOTSA_LOG_WARN("Invalid argument '{}', expected format --dump-hooks-to=path", arg);
        }
    } else {
        NOTSA_LOG_WARN("Unknown argument '{}'", arg);
    }
}

void CommandLine::Load(int argc, char** argv) {
    NOTSA_LOG_DEBUG("[Command Line]: Loading arguments");
    s_ExePath = argv[0];
    for (int32 i = 1; i < argc; i++) { // Skip first 'argument' which is the exec path.
        ProcessArgument(argv[i]);
    }
    NOTSA_LOG_DEBUG("[Command Line]: Arguments loaded");
}

fs::path GetExePath() {
    return s_ExePath;
}

void ApplyHookSettings() {
    using namespace ReversibleHooks;

    auto* const rh        = &RHManager::GetInstance();

    const auto ResultText = [](RHManager::SetCatOrItemStateResult res) {
        switch (res) {
        case RHManager::SetCatOrItemStateResult::NotFound: return "not found";
        case RHManager::SetCatOrItemStateResult::Locked:   return "locked";
        case RHManager::SetCatOrItemStateResult::Done:     return "done";
        default:                                NOTSA_UNREACHABLE();
        }
    };

    if (CommandLine::s_UnhookAll || !CommandLine::s_UnhookExcept.empty()) {
        rh->GetRootCategory().SetAllItemsState(ReversibleHooks::ReversibleHook::TwoWayHookState::RedirectToGTA);

        NOTSA_LOG_DEBUG("Unhooked all via command-line");
        for (const auto& item : CommandLine::s_UnhookExcept) {
            const auto res = rh->SetCategoryOrItemStateByPath(item, true);

            if (res == RHManager::SetCatOrItemStateResult::Done) {
                NOTSA_LOG_DEBUG("Rehooked '{}' via command-line.", item);
            } else {
                NOTSA_LOG_WARN("Couldn't rehook '{}' via command-line: {}", item, ResultText(res));
            }
        }
        return;
    }

    if (!CommandLine::s_UnhookSome.empty()) {
        for (const auto& item : CommandLine::s_UnhookSome) {
            const auto res = rh->SetCategoryOrItemStateByPath(item, false);

            if (res == RHManager::SetCatOrItemStateResult::Done) {
                NOTSA_LOG_DEBUG("Unhooked '{}' via command-line.", item);
            } else {
                NOTSA_LOG_WARN("Couldn't unhook '{}' via command-line: {}", item, ResultText(res));
            }
        }
    }
}

void DumpHooks() {
    if (const auto& path = CommandLine::s_DumpHooksPath; !path.empty()) {
        ReversibleHooks::RHManager::GetInstance().WriteHooksToFile(path);
    }
}

void PostHooksInjected() {
    ApplyHookSettings();
    DumpHooks();
}
}; // namespace CommandLine
