#include "StdInc.h"

#include <string>
#include <ranges>
#include <optional>
#include <format>
#include <charconv>

#include <imgui.h>
#include <libs/imgui/misc/cpp/imgui_stdlib.h>

#include <TristateCheckbox.h>

#include <extensions/utility.hpp>
#include <reversiblehooks/ReversibleHooks.h>
#include <reversiblehooks/HookCategory.h>

#include "Utility.h"
#include "HooksDebugModule.h"

namespace RH = ReversibleHooks;
namespace rng = std::ranges;
using HookState = ReversibleHooks::ReversibleHook::TwoWayHookState;

using namespace ImGui;

constexpr ImVec2 STATE_BUTTON_SIZE{ 80.f, 0.f };

// Clears both filters
// Making all items visible again is done by `DoFilter`
void HooksDebugModule::HookFilter::ClearFilters() {
    m_NamespaceTokens.clear(); // Clear only, so allocated memory is kept
    m_HookFilter = {};
}

// Are we filtering namespaces
bool HooksDebugModule::HookFilter::IsNamespaceFilterActive() {
    return !m_NamespaceTokens.empty();
}

// If empty it won't filter anything
bool HooksDebugModule::HookFilter::IsHookFilterEmpty() {
    return m_HookFilter->empty();
}

// Check if hook filter is present.
// even in case it's present it might be empty
// in which case it wouldn't filter out anything.
// Usually you want to use `IsHookFilterActive` which checks both.
bool HooksDebugModule::HookFilter::IsHookFilterPresent() {
    return m_HookFilter.has_value();
}

bool HooksDebugModule::HookFilter::IsHookFilterActive() {
    return IsHookFilterPresent() && !IsHookFilterEmpty();
}

// Are either filters active
bool HooksDebugModule::HookFilter::EitherFiltersActive() {
    return IsNamespaceFilterActive() || IsHookFilterActive();
}

// Should the current filtered namespace be relative to the root namespace.
// This is the case when the user prepends the namespace tokens with a `/` (NAMESPACE_SEP).
// Eg.: `/Entity` should only show the `Entity` namespace under `Root` (But not, for example, `Audio/AEVehicleAudioEntity`)
bool HooksDebugModule::HookFilter::IsRelativeToRootNamespace() {
    return m_NamespaceTokens.size() >= 1 && m_NamespaceTokens.front().empty();
}

// Make all categories and their items possibly visible and/or open
void HooksDebugModule::HookFilter::MakeAllVisibleAndOpen(ReversibleHooks::HookCategory& cat, bool visible, bool open) {
    cat.Visible(true);
    cat.Open(open);

    cat.m_anyItemsVisible = true;
    for (auto& i : cat.Items()) {
        i.SetMatchesSearchFilter(true);
    }

    for (auto& sc : cat.SubCategories()) {
        MakeAllVisibleAndOpen(sc, visible, open);
    }
}

// Returns `pair<visible, open>` of this category
auto HooksDebugModule::HookFilter::DoFilter_Internal(ReversibleHooks::HookCategory& cat, size_t depth) -> std::pair<bool, bool> {
    // Will be set to the appropriate values on return
    cat.Visible(false);
    cat.Open(false);

    const auto hasSubCategories = !cat.SubCategories().empty();

    // Process all sub-categories, and return if any category is pair<visible, open>
    const auto ProcessSubCategories = [&] {
        bool anyVisible{}, anyOpen{};
        for (auto& sc : cat.SubCategories()) {
            const auto [visible, open] = DoFilter_Internal(sc, depth + 1);
            anyVisible |= visible;
            anyOpen |= open;
        }
        return std::make_pair(anyVisible, anyOpen);
    };

    // If `doFilter` argument is `false` all items are set visible,
    // and either true (if we have hooks) or false (if `cat.Items().empty()`) is returned.
    // Otherwise items are filtered and true if returned if at least 1 item is visible.
    const auto ProcessItems = [&](bool allowFilter) {
        if (allowFilter && IsHookFilterActive()) {
            cat.m_anyItemsVisible = false;
            for (auto& i : cat.Items()) {
                auto matches = false;
                if (m_HookFilterByName) {
                    matches |= StringContainsString(i.GetName(), *m_HookFilter, m_IsCaseSensitive);
                }
                if (m_HookFilterByAddress) {
                    const auto CheckContainsAddress = [&](void* addr) {
                        char buf[64];
                        const auto [end, ec] = std::to_chars(buf, buf + sizeof(buf), (uintptr_t)(addr), 16);
                        if (ec != std::errc{}) {
                            return false;
                        }
                        return StringContainsString(std::string_view{ buf, end }, *m_HookFilter, false);
                    };
                    matches |= CheckContainsAddress(i.GetHookAddressGTA()) || CheckContainsAddress(i.GetHookAddressOur());
                }
                i.SetMatchesSearchFilter(matches);
                cat.m_anyItemsVisible |= matches;
            }
        } else { // Otherwise make sure all items are visible (if any)
            if (cat.Items().empty()) { // No items, make sure flag is set correctly.
                cat.m_anyItemsVisible = false;
            } else {
                cat.m_anyItemsVisible = true;
                for (auto& i : cat.Items()) {
                    i.SetMatchesSearchFilter(true);
                }
            }
        }
        return cat.m_anyItemsVisible;
    };

    if (IsNamespaceFilterActive()) {
        if (IsRelativeToRootNamespace()) { // Eg.: (Notice the trailing `/`) /Entity/Ped/Player/ (Should show `Root/Entity/Ped/CPlayerPed`)
            // Using root namespace.
            // In this case all tokens must match up to root

            const auto ProcessFilter = [&]() -> bool {
                if (depth == 0) { // Special case - Root namespace depth, no need to check any tokens
                    assert(m_NamespaceTokens.front().empty()); // This codepath should be unreachable unless first token is empty
                    return true;
                } else if (depth < m_NamespaceTokens.size()) {
                    if (   hasSubCategories                      // Unless we're last category to match we must have more sub-categories so all tokens can match
                        || depth == m_NamespaceTokens.size() - 1 // Last token to match, so there will be no more, thus it's fine if there are no more subcategories.
                        || m_NamespaceTokens.back().empty()      // Or last token is empty (Empty strings always match everything) - This way a trailing `/` opens the category (like `::` does)
                        ) {
                        return StringContainsString(cat.Name(), m_NamespaceTokens[depth], m_IsCaseSensitive);
                    } else {
                        return false; // Not enough children to statify all tokens
                    }
                } else { // Our parents fully matched the tokens, they just want to make us visible, but not open
                    return true;
                }
            };

            const auto byFilterVisible = ProcessFilter();
            if (!byFilterVisible) {
                return {};
            }

            // If botom level, and hook filtering is present: make us open, and dont show any more sub-categories
            if (depth == m_NamespaceTokens.size() - 1 /*bottom level*/ && IsHookFilterPresent()) {
                MakeAllVisibleAndOpen(cat, false, false);

                (void)ProcessItems(true);

                cat.Visible(true);
                cat.Open(true);

                return { true, true };
            }

            const auto [anySCVisible, anySCOpen] = ProcessSubCategories();

            const bool open    = anySCOpen || depth + 1 < m_NamespaceTokens.size() /*All categories before the bottom level should be open*/;
            const bool visible = anySCVisible || byFilterVisible;

            cat.Visible(visible);
            cat.Open(open);

            return { visible, open };
        } else { // Eg.: Ped/Player/ (Should show `Root/Entity/Ped/CPlayerPed`)
            // Not in the root namespace
            // All tokens should match in reverse order starting at us, eg.:
            // Entity::Ped::Ped
            // So, in order to be visible* `cat`s name should contain `Ped`
            // it's parent's name should contain `Ped` and it's parent's name should contain `Entity`
            // *We may be visible if there are subcategories visible even if this function returns false

            // Example:
            // Parents: Root-Entity-Ped-CPed
            // Depth:   0     1       2    3   <= Also number of tokens
            // Tokens:        Entity::Ped::Ped

            const auto ProcessFilter = [&]() -> bool {
                if (depth < m_NamespaceTokens.size()) { // Optimization: Not enough categories to possibly staisfy all tokens
                    return false;
                }

                for (auto icat{ &cat }; auto&& token : m_NamespaceTokens | rng::views::reverse) {
                    if (!StringContainsString(icat->Name(), token, m_IsCaseSensitive)) { // Couldn't statify all tokens - Remember: `contains` always returns true if `token.empty()`
                        return false;
                    }
                    icat = icat->Parent();
                }

                return true;
            };

            const auto byFilterVisible           = ProcessFilter();
            const auto itemsVisible              = ProcessItems(true); // Filter items
            const auto [anySCVisible, anySCOpen] = ProcessSubCategories();

            const bool open    = anySCOpen || anySCVisible || (hasSubCategories && byFilterVisible) || (IsHookFilterPresent() && itemsVisible);
            const bool visible = (byFilterVisible && itemsVisible) || anySCVisible;

            cat.Visible(visible);
            cat.Open(open);

            return { visible, open };
        }
    } else {
        // Filter by hook names
        // Category is visible if it:
        // - It has visible hooks (After filtering)
        // - Or it has visible sub-categories

        const auto itemsVisible = ProcessItems(true); // Filter items
        const auto [anySubCatVisible, anySubCatOpen] = ProcessSubCategories();

        const auto open    = itemsVisible || anySubCatOpen;
        const bool visible = open || anySubCatVisible;

        cat.Visible(visible);
        cat.Open(open);

        return { visible, open };
    }
}

void HooksDebugModule::HookFilter::DoFilter(RH::HookCategory& cat) {
    if (EitherFiltersActive()) {
        DoFilter_Internal(cat);
    } else {
        MakeAllVisibleAndOpen(cat, true, false); // Make all visible, but closed
    }
}

void HooksDebugModule::HookFilter::OnInputUpdate() {
    const std::string_view inputsv{ m_Input };

    ClearFilters();

    if (!inputsv.empty()) { 
        // Extract namespace tokens and the hook name filter
        {
            const auto sepPos = inputsv.rfind(HOOK_FILTER_SEP);

            // First half contains the namespace filter tokens
            for (auto t : SplitStringView(inputsv.substr(0, sepPos), NAMESPACE_SEP)) {
                m_NamespaceTokens.emplace_back(t);
            }

            // Second half (if any) contains the hook/function name filter
            if (sepPos != std::string_view::npos) {
                const auto filter     = notsa::trim_string(inputsv.substr(sepPos + HOOK_FILTER_SEP.size()));
                m_HookFilter          = filter;
                m_HookFilterByName    = !filter.starts_with("0x");
                m_HookFilterByAddress = !m_HookFilterByAddress || notsa::try_ston<uintptr>(filter, 16).has_value();
            }
        }

        // In case user passes in a string with multiple `/` with nothing in-between we will have quite a few empty tokens.
        // We have have to remove all the leading empty tokens up until the last empty one.
        while (m_NamespaceTokens.size() >= 2 && m_NamespaceTokens[0].empty() && m_NamespaceTokens[1].empty()) {
            m_NamespaceTokens.erase(m_NamespaceTokens.begin());
        }

        // Don't delete this please //
        //
        /*std::cout << "update input: ";
        for (auto&& t : m_namespaceTokens) {
        std::cout << '`' << t << "`,";
        }
        std::cout << ";" << m_hookFilter.value_or("None") << "\n";*/

        // If we're using the root namespace only but there's no hook filter we practically don't filter anything
        // Example user inputs: `/`, `/::`, `::`
        if (IsRelativeToRootNamespace() && m_NamespaceTokens.size() == 1 && IsHookFilterActive()) {
            ClearFilters();
        }
    }

    DoFilter(RH::GetRootCategory()); 
}

void HooksDebugModule::HookFilter::Render() {
    PushItemWidth(GetWindowContentRegionMax().x - 10.f);
    if (InputText(" ", &m_Input)) {
        OnInputUpdate();
    }
    if (IsItemHovered()) {
        SetTooltip(
            "`::function`         - Filters only functions \n"
            "`cpy`                - Filter namespace - Will only show namespace with name containing \"cphy\"\n"
            "`ped/player`         - Should only show Ped/CPlayerPed\n"
            "`ped/player::busted` - Should only show `Ped/CPlayerPed` with the `busted` function visible only\n"
            "`/entity`            - Should only show the top level `Entity` namespace in Root\n"
            "For more tips see gta-reversed-modern/discussions/190\n"
        );
    }
    PopItemWidth();
}

bool HooksDebugModule::HandleSlideSetterForItem(bool& inOutState) {
    if (m_SlideSetter.Mode == SlideSetterMode::NONE || !IsItemHovered()) {
        return false;
    }
    if (m_SlideSetter.Mode == SlideSetterMode::SETTER) {
        m_SlideSetter.Mode = inOutState
            ? SlideSetterMode::TURN_ON
            : SlideSetterMode::TURN_OFF;
    }
    inOutState = [&]{
        switch (m_SlideSetter.Mode) {
        case SlideSetterMode::TURN_ON:  return true;
        case SlideSetterMode::TURN_OFF: return false;
        case SlideSetterMode::TOGGLE:   return !inOutState;
        default: NOTSA_UNREACHABLE();
        }
    }();
    return true;
}

const char* StateToString(HookState state) {
    switch (state) {
    case HookState::Unhooked:       return "unhooked";
    case HookState::RedirectToGTA:  return "gta";
    case HookState::RedirectToOurs: return "our";
    default:                        return "unk";
    }
}

void StateButton(
    const char*              title,
    bool                     disabled,
    ImTristate               onOffCheckboxState,
    std::optional<HookState> current,
    HookState                next,
    auto&&                   SetState,
    auto&&                   RestoreState
) {
    notsa::ui::ScopedID      idg{ "state" };
    notsa::ui::ScopedDisable sdg{ disabled };

    SameLine(); 
    if (bool checked; CheckboxTristate("##on-off", onOffCheckboxState, checked)) {
        if (checked) {
            RestoreState();
        } else {
            SetState(HookState::Unhooked);
        }
    }

    const auto a = (int32)(GetStyle().Colors[ImGuiCol_Button].w * 255.f);
    PushStyleColor(
        ImGuiCol_Button,
        current.transform([a] (HookState state) {
            switch (state) {
            case HookState::Unhooked:       return IM_COL32(127, 0, 0, a);  // Red
            case HookState::RedirectToGTA:  return IM_COL32(69, 69, 69, a); // Dark gray
            case HookState::RedirectToOurs: return IM_COL32(0, 127, 0, a);  // Green
            default:                        NOTSA_UNREACHABLE_CASE(state);
            }
        }).value_or(IM_COL32(127, 127, 0, a))
    );

    SameLine(); 
    if (Button(current.has_value() ? StateToString(*current) : "mixed", STATE_BUTTON_SIZE) && !disabled) {
        SetState(next);
    }
    PopStyleColor();

    SameLine();
    TextUnformatted(title);

}

auto GetNextCycleState(std::optional<HookState> last, bool withUnhooked = false) noexcept {
    switch (const auto value = last.value_or(HookState::RedirectToOurs)) { // const auto last = last.value_or(OverallState().value_or(HookState::RedirectToOurs))
    case HookState::RedirectToOurs: return HookState::RedirectToGTA;
    case HookState::RedirectToGTA:  return withUnhooked ? HookState::Unhooked : HookState::RedirectToOurs;
    case HookState::Unhooked:       return HookState::RedirectToOurs;
    default:                        NOTSA_UNREACHABLE_CASE(value);
    }
}

void HooksDebugModule::RenderCategoryItems(RH::HookCategory& cat) {
    for (auto& item : cat.Items()) {
        if (!item.GetMatchesSearchFilter()) {
            continue;
        }

        notsa::ui::ScopedID idg{ item.GetName() };

        // Draw hook symbol
        {
            PushStyleVar(ImGuiStyleVar_Alpha, GetStyle().Alpha * 0.5f);
            AlignTextToFramePadding();
            Text(item.GetTypeSymbolUI());
            PopStyleVar();
        }

        // State checkbox
        {
            StateButton(
                item.GetName().c_str(),
                item.GetIsStateLocked(),
                item.GetState() == HookState::Unhooked
                    ? ImTristate::NONE
                    : ImTristate::ALL,
                item.GetState(),
                item.GetState() == HookState::RedirectToOurs
                    ? HookState::RedirectToGTA
                    : HookState::RedirectToOurs,
                [&](HookState s) { cat.SetItemState(item, s); },
                [&]() { cat.SetItemState(item, item.GetPreviousState()); }
            );

            //IDScope("state");
            //DisabledScope(item.GetIsStateLocked());


            //SameLine(); 
            //bool checked        = item.GetState() != HookState::Unhooked;
            //if (Checkbox("##on-off", &checked)) {
            //    cat.SetItemState(item, checked ? item.GetPreviousState() : HookState::Unhooked);
            //}
            //
            //SameLine();
            //const auto next = item.GetState() == HookState::RedirectToOurs
            //    ? HookState::RedirectToGTA
            //    : HookState::RedirectToOurs;
            //if (Button(StateToString(item.GetState()), STATE_BUTTON_SIZE) && !item.GetIsStateLocked()) {
            //    cat.SetItemState(item, next);
            //}
            //if (!item.GetIsStateLocked() && IsItemHovered()) {
            //    SetTooltip("Change to: %s", StateToString(next));
            //}
            //
            //SameLine();
            //TextUnformatted(item.GetName().c_str());


            //if (SameLine(); CheckboxTristate(item.GetName().c_str(), item.GetStateUI(), checked) && !item.GetIsStateLocked()) { 
            //    cat.SetItemState(item,
            //        IsKeyDown(ImGuiMod_Alt)
            //            ? HookState::Unhooked
            //            : checked
            //                ? HookState::RedirectToOurs
            //                : HookState::RedirectToGTA
            //    );
            //}
            //if (IsItemHovered()) {
            //    SetTooltip(
            //        "Currently state: %s"
            //        "Left click: Redirect to Our/GTA code\n"
            //        "Left click + Alt: Unhook\n"
            //        "Middle click: Toggle (Slide setter)\n"
            //        "Right click + hold: Slide setter (Enable/disable all hovered items)\n",
            //        EnumToString(item.GetState()).value_or("Unknown")
            //    );
            //}
            //if (!item.GetIsStateLocked() && HandleSlideSetterForItem(checked)) {
            //    cat.SetItemState(item, checked ? HookState::RedirectToOurs : HookState::RedirectToGTA);
            //}
        }

        if (!IsItemHovered()) {
            continue;
        }

        const auto gta = item.GetHookAddressGTA(),
                   our = item.GetHookAddressOur();
        if (gta && our) {
            const auto AddrToClipboard = [](void* addr) {
                SetClipboardText(std::format("{}", addr).c_str());
            };

            std::string tooltipText = std::format("SA: {} / Our: {}", gta, our);
            if (item.GetIsStateLocked()) {
                tooltipText += "\n(locked)";
            }
            SetTooltip(tooltipText.c_str());

            if (IsItemClicked(ImGuiMouseButton_Right)) {
                AddrToClipboard(gta);
            } else if (IsItemClicked(ImGuiMouseButton_Middle)) {
                AddrToClipboard(our);
            }
        }
    }
}

void HooksDebugModule::RenderCategory(RH::HookCategory& cat) {
    if (!cat.Visible()) {
        return;
    }
    notsa::ui::ScopedID idg{ cat.Name() };

    const auto TreeNodeWithCheckbox = [](
        auto                                            label,
        bool                                            disabled,
        bool                                            hasAnyUnhooked,
        ReversibleHooks::HookCategory::CommonItemsState commonState,
        HookState                                       next,
        auto&&                                          SetState,
        auto&&                                          RestoreState
    ) {
        // TODO/NOTE: The Tree's label is a workaround for when the label is shorter than the visual checkbox (otherwise the checkbox can't be clicked)
        const auto open = TreeNodeEx("##         ", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth);
        
        SameLine();
        StateButton(
            label,
            disabled,
            commonState == HookState::Unhooked
                ? ImTristate::NONE
                : hasAnyUnhooked
                    ? ImTristate::MIXED
                    : ImTristate::ALL,
            commonState,
            next,
            SetState,
            RestoreState
        );

        // Tree is never disabled, otherwise it couldn't be opened
        //DisabledScope(disabled);
        //
        //// Checkbox + it's label will be the tree name
        //bool cbState{ commonState.has_value() };
        //const auto stateChanged = Checkbox(label, &cbState);
        //
        //if (SameLine(); Button(commonState.has_value() ? StateToString(*commonState) : "mixed")) {
        //    cbState = !cbState;
        //}

        //if (IsItemHovered()) {
        //    SetTooltip(
        //        "Left click: Redirect to Our/GTA code\n"
        //        "Left click + Alt: Unhook\n"
        //        "Middle click: Toggle (Slide setter)\n"
        //        "Right click + hold: Slide setter (Enable/disable all hovered items)\n"
        //    );
        //}

        return std::make_tuple(open, false);
    };

    // Disable all hooks in category at once
    {
        SetNextItemOpen(cat.Open());


        const auto [open, stateChanged] = TreeNodeWithCheckbox(
            cat.Name().c_str(),
            cat.Disabled(),
            cat.HasAnyUnhooked(),
            cat.OverallState(),
            cat.OverallState().value_or(cat.m_LastSetAllState.value_or(HookState::RedirectToOurs)) == HookState::RedirectToOurs
                ? HookState::RedirectToGTA
                : HookState::RedirectToOurs,
            [&](HookState s) { cat.SetAllItemsState(s); },
            [&] () { cat.SetAllItemsToPreviousState(); }
        );
        //if (stateChanged) {
        //    cat.ToggleAllItemsState();
        //}
        //if (bool state = cbState; HandleSlideSetterForItem(state)) {
        //    cat.SetAllItemsState(state);
        //}
        cat.Open(open);
    }

    if (!cat.Open()) {
        return;
    }

    //
    // Draw hooks, and subcategories
    //

    // Draw hooks (items) (if any)
    if (!cat.Items().empty() && cat.m_anyItemsVisible) {
        if (cat.SubCategories().empty()) { // If there are no subcategories we can draw all items directly
            RenderCategoryItems(cat);
        } else { // Otherwise use a tree node + checkbox for them
            const auto [open, stateChanged] = TreeNodeWithCheckbox(
                "Hooks",
                cat.ItemsDisabled(),
                cat.m_AnyOurItemsUnhooked,
                cat.ItemsState(),
                cat.ItemsState().value_or(cat.m_LastSetOurState.value_or(HookState::RedirectToOurs)) == HookState::RedirectToOurs
                    ? HookState::RedirectToGTA
                    : HookState::RedirectToOurs,
                [&](HookState s) { cat.SetOurItemsState(s); },
                [&] () { cat.SetOurItemsToPreviousState(); }
            );

            if (stateChanged) {
                //cat.SetOurItemsState(cbState);
                //cat.ToggleAllItemsState();
                NOTSA_UNREACHABLE("todo");
            }

            if (open) {
                RenderCategoryItems(cat);
                TreePop();
            }
        }
    }

    // Draw subcategories
    for (auto& v : cat.SubCategories()) {
        RenderCategory(v);
    }

    TreePop();
}

void HooksDebugModule::RenderWindow() {
    const notsa::ui::ScopedWindow window{ "ReversibleHooks (TM) (R)", {500.f, 700.f}, m_IsOpen, ImGuiWindowFlags_MenuBar };
    if (!m_IsOpen) {
        return;
    }
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Export hooks.csv")) {
                const auto path = fs::weakly_canonical("hooks.csv");
                ReversibleHooks::WriteHooksToFile(path);
                NOTSA_LOG_INFO("Exported hooks to {:?}", path.string());
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    m_SlideSetter.Mode = IsMouseDown(ImGuiMouseButton_Middle)
        ? SlideSetterMode::TOGGLE
        : IsMouseDown(ImGuiMouseButton_Right)
            ? m_SlideSetter.Mode == SlideSetterMode::TURN_OFF || m_SlideSetter.Mode == SlideSetterMode::TURN_ON
                ? m_SlideSetter.Mode // Technically in setter mode already
                : SlideSetterMode::SETTER
            : SlideSetterMode::NONE;
    m_HookFilter.Render();
    RenderCategory(RH::GetRootCategory());
}

void HooksDebugModule::RenderMenuEntry() {
    notsa::ui::DoNestedMenuIL({ "Settings" }, [&] {
        ImGui::MenuItem("Hooks", nullptr, &m_IsOpen);
    });
}
