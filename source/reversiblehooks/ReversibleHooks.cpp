#include "StdInc.h"
#include <unordered_set>
#include <extensions/CommandLine.h>

#ifdef NOTSA_WITH_SCRIPT_COMMAND_HOOKS
#include "ReversibleHook/ScriptCommandHook.h"
#endif
#include "ReversibleHooks.h"
#include "ReversibleHook/BasicHook.h"
#include "ReversibleHook/VirtualHook.h"
#include "RootHookCategory.h"
#include "VMTInfo.h"
#include <fstream>

namespace ReversibleHooks {

RootHookCategory s_RootCategory{};
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
        if (item->Hooked() == enabled) {
            return SetCatOrItemStateResult::Done;
        }
        return item->State(enabled) ? SetCatOrItemStateResult::Done : SetCatOrItemStateResult::Locked;
    }

    return SetCatOrItemStateResult::NotFound;
}

void CheckAll() {
    s_RootCategory.ForEachItem([](auto& item) {
        item->Check();
    });
}

void SwitchHook(std::string_view funcName) {
    s_RootCategory.ForEachItem([=](auto& item) {
        const auto name = item->Name();
        if (name == funcName) {
            item->Switch();
            return;
        }
    });
}

void OnInjectionBegin(HMODULE hThisDLL) {
    NOTSA_LOG_DEBUG("OnInjectionBegin");
}

void OnInjectionEnd() {
    NOTSA_LOG_DEBUG("OnInjectionBegin");
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
    auto item = std::make_shared<ReversibleHook::Virtual>(
        std::move(fnName),
        vmtInfoOur.GetEntryAddressAt(idx),
#ifdef NOTSA_STANDALONE
        nullptr,
#else
        vmtInfoGTA.GetEntryAddressAt(idx),
#endif
        opt.reversed
    );
    item->State(opt.enabled);
    item->LockState(opt.locked);
    AddItemToCategory(category, std::move(item));
}

void AddItemToCategory(std::string_view category, std::shared_ptr<ReversibleHook::BaseHook> item) {
    s_RootCategory.AddItemToNamedCategory(category, std::move(item));
}

#ifdef NOTSA_WITH_SCRIPT_COMMAND_HOOKS
void InstallScriptCommand(std::string_view category, eScriptCommands cmd) {
    AddItemToCategory(
        category,
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
                if (item->Type() == BaseHook::HookType::ScriptCommand) {
                    continue;
                }
                const auto address = [&] {
                    switch (item->Type()) {
                    case BaseHook::HookType::Virtual:
                        return std::static_pointer_cast<Virtual>(item)->GetHookGTAAddress();
                    case BaseHook::HookType::Basic:
                        return std::static_pointer_cast<BasicHook>(item)->GetHookGTAAddress();
                    default:
                        NOTSA_UNREACHABLE();
                    }
                }();

                std::println(
                    of,
                    "{},{},0x{:08X},{},{},{}",
                    cat.Name(),
                    item->Name(),
                    (uintptr_t)address,
                    (int32)item->Reversed(),
                    (int32)item->Locked(),
                    item->Symbol()
                );
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

    auto item = std::make_shared<ReversibleHook::BasicHook>(
        std::move(fnName),
        (uint32)installAddress,
        addressToJumpTo,
        opt.reversed,
        opt.jmpCodeSize,
        opt.stackArguments
    );
    
    item->State(opt.enabled);
    item->LockState(opt.locked);
    AddItemToCategory(category, std::move(item));
}
}; // namespace detail
}; // namespace ReversibleHooks
