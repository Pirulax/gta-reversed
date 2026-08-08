#include "StdInc.h"
#include <unordered_set>
#include <extensions/CommandLine.h>

#ifdef NOTSA_WITH_SCRIPT_COMMAND_HOOKS
#include "ReversibleHook/ScriptCommandHook.h"
#endif
#include "ReversibleHooks.h"
#include "ReversibleHook/StaticHook.h"
#include "ReversibleHook/VirtualHook.h"
#include "RootHookCategory.h"
#include "VMTInfo.h"
#include <fstream>


ReversibleHooks::RootHookCategory s_RootCategory{};

using HooksCheckClock = std::chrono::steady_clock;
static constexpr auto HOOKS_CHECK_INTERVAL = std::chrono::milliseconds{ 500 };
HooksCheckClock::time_point s_LastHooksCheckTime{};

namespace ReversibleHooks {
RootHookCategory& GetRootCategory() {
    return s_RootCategory;
}

SetCatOrItemStateResult SetCategoryOrItemStateByPath(std::string_view path, bool enabled) {
    if (path.ends_with("/")) {
        path.remove_suffix(1);
    }

    const auto    separated = SplitStringView(path, "/") | rng::to<std::vector>();
    HookCategory* cat       = &GetRootCategory();
    for (auto name : std::span(separated).first(separated.size() - 1)) {
        cat = cat->FindSubcategory(name);
        if (!cat) {
            return SetCatOrItemStateResult::NotFound;
        }
    }

    if (auto category = cat->FindSubcategory(separated.back())) {
        category->SetAllItemsEnabled(enabled);
        return SetCatOrItemStateResult::Done;
    } else if (auto item = cat->FindItem(separated.back())) {
        if (item->GetState() == enabled) {
            return SetCatOrItemStateResult::Done;
        }
        return item->SetState(enabled) ? SetCatOrItemStateResult::Done : SetCatOrItemStateResult::Locked;
    }

    return SetCatOrItemStateResult::NotFound;
}

void CheckAll() {
    if (const auto now = HooksCheckClock::now(); now - s_LastHooksCheckTime > HOOKS_CHECK_INTERVAL) {
        s_LastHooksCheckTime = now;
        s_RootCategory.ForEachItem([](HookCategoryItem& item) {
            item.GetHook()->Check();
        });
    }
}

void OnInjectionBegin(HMODULE hThisDLL) {
    NOTSA_LOG_DEBUG("OnInjectionBegin");
}

void OnInjectionEnd() {
    NOTSA_LOG_DEBUG("OnInjectionEnd");
    s_RootCategory.OnInjectionEnd();
    if (!CommandLine::s_DumpHooksPath.empty()) {
        WriteHooksToFile(CommandLine::s_DumpHooksPath);
    }
}

void InstallVirtual(
    std::string_view   category,
    std::string        fnName,
    Utility::VMTInfo   vmtInfoOur,
    void*              fnAddressOur,
    Utility::VMTInfo   vmtInfoGTA,
    void*              fnAddressGTA,
    HookInstallOptions opt
) {
    const auto idx = vmtInfoGTA.FindIndexOf(fnAddressGTA);
    // We can't do `vmtInfoOur.FindIndexOf(fnAddressOur)` because `fnAddressOur` points to the thunk, while the address in the VMT is pointing to the actual function
    AddHookToCategory(category, opt, std::make_shared<ReversibleHook::VirtualHook>(
        std::move(fnName),
        vmtInfoOur.GetEntryAddressAt(idx),
#ifdef NOTSA_STANDALONE
        nullptr
#else
        vmtInfoGTA.GetEntryAddressAt(idx)
#endif
    ));
}
    
void AddHookToCategory(std::string_view category, HookInstallOptions opt, std::shared_ptr<ReversibleHook::TwoWayHookBase> hook) {
    s_RootCategory.AddItemToNamedCategory(category, { std::move(hook), opt.locked, opt.reversed, opt.enabled, opt.InstallSrcLoc });
}

#ifdef NOTSA_WITH_SCRIPT_COMMAND_HOOKS
void InstallScriptCommand(std::string_view category, eScriptCommands cmd, HookInstallOptions opt) {
    AddHookToCategory(
        category,
        opt,
        std::make_shared<::ReversibleHooks::ReversibleHook::ScriptCommandHook>(cmd)
    );
}
#endif

void WriteHooksToFile(const std::filesystem::path& file) {
    const auto path = std::filesystem::weakly_canonical(file);
    if (std::ofstream of{ file }) {
        of << "class,fn_name,address,reversed,locked,type\n";
        s_RootCategory.ForEachCategory([&](const HookCategory& cat) {
            using namespace ReversibleHook;
            for (const auto& item : cat.Items()) {
                if (item.GetType() == TwoWayHookBase::HookType::ScriptCommand) {
                    continue;
                }
                item.PrintToCSV(of, cat);
            }
        });
        NOTSA_LOG_INFO("Hooks written to `{}`", path.string());
    } else {
        NOTSA_LOG_ERR("Failed to open file `{}` for writing hooks!", path.string());
    }
}

namespace detail {
std::unordered_set<void*> s_HookedAddresses{};  // Original GTA addresses to which we've installed hooks
bool MarkAddressAsHooked(void* address) {
    const auto [iter, inserted] = s_HookedAddresses.insert(address);
    return inserted;
}

void HookInstall(std::string_view category, std::string fnName, void* installAddress, void* addressToJumpTo, HookInstallOptions&& opt) {
#ifndef NDEBUG // Functions with the same name are asserted in `HookCategory::AddItem()`
    if (!MarkAddressAsHooked(installAddress)) {
        throw std::runtime_error(std::format("{}/{} is hooked to an address ({}) that is already hooked! That's bad!", category, fnName, installAddress));
    }
#endif

    // If `PreserveRegisters` then `StackArgumentsToPreserve` may be 0, we just want to enforce it to be set to prevent bugs
    if (opt.PreserveRegisters && !opt.StackArgumentsToPreserve.has_value()) {
        throw std::runtime_error(std::format("{}/{}: `PreserveRegisters` requires `StackArgumentsToPreserve` to be set!", category, fnName));
    }

    AddHookToCategory(category, opt, std::make_shared<ReversibleHook::StaticHook>(
        std::move(fnName),
        addressToJumpTo,
        installAddress,
        opt.StackArgumentsToPreserve.value_or(0),
        opt.PreserveRegisters
    ));
}
}; // namespace detail
}; // namespace ReversibleHooks
