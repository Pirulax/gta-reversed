#include "StdInc.h"

#include <string>
#include <chrono>
#include <optional>
#include <format>

#include <imgui.h>
#include <libs/imgui/misc/cpp/imgui_stdlib.h>

#include <TristateCheckbox.h>

#include <extensions/utility.hpp>
#include <reversiblehooks/ReversibleHooks.h>
#include <reversiblehooks/HookCategory.h>

#include "Utility.h"
#include "HooksDebugModule.h"

#include "HooksSortListStep.hpp"

namespace RH = ReversibleHooks;
namespace rng = std::ranges;
using HookState = ReversibleHooks::ReversibleHook::TwoWayHookState;

using namespace ImGui;

constexpr ImVec2 STATE_BUTTON_SIZE{ 80.f, 0.f };

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


void RHDebugModule::HooksDebugModule::FilteringThread() {
    while (!m_FilterProcessor.Exiting) {
        std::unique_lock lock{ m_FilterProcessor.Mtx };
        m_FilterProcessor.CV.wait(lock);
        if (m_FilterProcessor.Exiting) {
            break;
        }
        if (!m_ToRender) {
            continue; // Nothing to filter
        }
        /* Hold lock until we finish */
        NOTSA_LOG_DEBUG("Running filter");
        const auto now = FilterClock::now();
        HooksFilterListStep{ std::move(m_FilterProcessor.HookFilter) }.Process(*m_ToRender);
        HooksSortListStep{}.Process(*m_ToRender);
        m_FilterProcessor.TimeToFinish = FilterClock::now() - now;
        m_FilterProcessor.DidJustFinish = true;
    }
}

bool RHDebugModule::HooksDebugModule::RunFilter() {
    if (!m_ToRender) {
        return false; // Nothing to filter
    }
    {
        std::unique_lock lock{ m_FilterProcessor.Mtx, std::try_to_lock };
        if (!lock.owns_lock()) {
            return false; // Filter still running
        }
        m_FilterProcessor.HookFilter = { m_Filter.Input, m_Filter.CaseSensitive, m_Filter.Cutoffs };
    }
    m_FilterProcessor.CV.notify_one();
    return true;
    //    if (m_FilterTask.valid() && m_FilterTask.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
    //return false; // Previous task still running, it has to finish first, because data is shared
    //    }
    //
    //    m_FilterTask = std::async([this, filter = HookFilter{ m_FilterInput, m_FilterCutoffs }] mutable {
    //#ifdef TRACY_ENABLE
    //tracy::SetThreadName("HooksDebugModule::RunFiltering");
    //#endif
    //HooksFilterListStep{ std::move(filter) }.Process(*m_ToRender);
    //HooksSortListStep{}.Process(*m_ToRender);
    //    });
    //
    //    return true;
}

void RHDebugModule::HooksDebugModule::RenderFilter() {
    notsa::ui::ScopedID idg{ "Filter" };

    bool changed = false;

    if (TreeNode("Filter Options")) {
        changed |= Checkbox("Show Filter Scores", &m_Filter.ShowScores);
        changed |= Checkbox("Case Sensitive", &m_Filter.CaseSensitive);
        if (TreeNode("Cutoffs")) {
            const auto CutoffSlider = [&](float* value, const char* name) {
                changed |= SliderFloat(name, value, 0.f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            };
            CutoffSlider(&m_Filter.Cutoffs.Category, "Category");
            CutoffSlider(&m_Filter.Cutoffs.CategoryGlobal, "CategoryGlobal");
            CutoffSlider(&m_Filter.Cutoffs.ItemGlobal, "ItemGlobal");
            CutoffSlider(&m_Filter.Cutoffs.ItemLocal, "ItemLocal");
            CutoffSlider(&m_Filter.Cutoffs.ItemAddress, "ItemAddress");

            TreePop();
        }

        TreePop();
    }

    SetNextItemWidth(-1.f);
    changed |= InputText("##Input", &m_Filter.Input);
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
    
    if (changed) {
        m_Filter.RunAt = FilterClock::now() + FILTER_INPUT_DEBOUNCE_TIME;
    } else if (m_Filter.RunAt.has_value() && *m_Filter.RunAt < FilterClock::now()) {
        if (RunFilter()) {
            m_Filter.RunAt = std::nullopt;
        }
    }
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

bool IsMatchingScoreOrNone(const std::optional<float>& score, float cutoff = 0.f) {
    return !score.has_value() || *score > cutoff;
}

void RHDebugModule::HooksDebugModule::UpdateSlideSetterMode() {
    m_SlideSetter.Mode = IsMouseDown(ImGuiMouseButton_Middle)
        ? SlideSetterMode::TOGGLE
        : IsMouseDown(ImGuiMouseButton_Right)
            ? m_SlideSetter.Mode == SlideSetterMode::TURN_OFF || m_SlideSetter.Mode == SlideSetterMode::TURN_ON
                ? m_SlideSetter.Mode // Technically in setter mode already
                : SlideSetterMode::SETTER
            : SlideSetterMode::NONE;

}

void RHDebugModule::HooksDebugModule::RenderMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Export hooks.csv")) {
                const auto path = fs::weakly_canonical("hooks.csv");
                ReversibleHooks::RHManager::GetInstance().WriteHooksToFile(path);
                NOTSA_LOG_INFO("Exported hooks to {:?}", path.string());
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

// Should acquire lock on `cat.ItemsMtx` before calling
bool HooksDebugModule::RenderCategoryItems(StepsCategory& cat) {
    if (!IsMatchingScoreOrNone(cat.MaxFilterScoreOwnItems)) {
        return false;
    }
    for (auto& item : cat.Items) {
        if (!IsMatchingScoreOrNone(item.FilterScore)) {
            continue;
        }
        notsa::ui::ScopedID idg{ item.Ptr->GetName() };

        // Draw hook symbol
        {
            PushStyleVar(ImGuiStyleVar_Alpha, GetStyle().Alpha * 0.5f);
            AlignTextToFramePadding();
            Text("T"); //Text(item.GetTypeSymbolUI());
            PopStyleVar();
        }

        // State checkbox
        StateButton(
            m_Filter.ShowScores
                ? std::format("{} (Score: {})", item.Ptr->GetName(), item.FilterScore.value_or(-1.f)).c_str()
                : item.Ptr->GetName().c_str(),
            item.Ptr->GetIsStateLocked(),
            item.Ptr->GetState() == HookState::Unhooked
                ? ImTristate::NONE
                : ImTristate::ALL,
            item.Ptr->GetState(),
            item.Ptr->GetState() == HookState::RedirectToOurs
                ? HookState::RedirectToGTA
                : HookState::RedirectToOurs,
            [&] (HookState s) { item.Ptr->SetState(s); },
            [&] () { item.Ptr->SetToPreviousState(); }
        );

        if (IsItemHovered()) {
            const auto gta = item.Ptr->GetHookAddressGTA(),
                       our = item.Ptr->GetHookAddressOur();
            const auto AddrToClipboard = [](void* addr) {
                SetClipboardText(std::format("{}", addr).c_str());
            };

            std::string tooltipText = std::format("SA: {} / Our: {}", gta, our);
            if (item.Ptr->GetIsStateLocked()) {
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
    return true;
}

bool HooksDebugModule::RenderCategory(StepsCategory& cat) {
    if (!IsMatchingScoreOrNone(cat.MaxFilterScore)) {
        return false;
    }

    const auto hasItemsToShow      = cat.HasItems && IsMatchingScoreOrNone(cat.MaxFilterScoreOwnItems),
               hasCategoriesToShow = cat.HasSubCategories && IsMatchingScoreOrNone(cat.MaxFilterScoreSubCats);

    notsa::ui::ScopedID idg{ cat.Category->Name() };

    const auto TreeNodeWithCheckbox = [](
        const char*                                     label,
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

    // Category tree node
    if (m_FilterProcessor.DidJustFinish) {
        if (cat.MaxFilterScore > 0.f) {
            SetNextItemOpen(true, ImGuiCond_Always);
        }
    }

    const auto [open, stateChanged] = TreeNodeWithCheckbox(
        m_Filter.ShowScores
            ? std::format(
                  "{} Max filter scores: {{Own: {:.2f}, OwnItems: {:.2f}, SubItems: {:.2f}, AllItems: {:.2f}, SubCats: {:.2f}, All: {:.2f}}}",
                  cat.Category->Name(),
                  cat.FilterScore.value_or(-1.f),
                  cat.MaxFilterScoreOwnItems.value_or(-1.f),
                  cat.MaxFilterScoreSubItems.value_or(-1.f),
                  cat.MaxScoreAllItems.value_or(-1.f),
                  cat.MaxFilterScoreSubCats.value_or(-1.f),
                  cat.MaxFilterScore.value_or(-1.f)
              ).c_str()
            : cat.Category->Name().c_str(),
        !cat.AnyUnlockedItems,
        cat.AnyUnhookedItems,
        cat.CommonStateAllItems,
        //cat.CommonStateAllItems.value_or(cat.m_LastSetAllState.value_or(HookState::RedirectToOurs)) == HookState::RedirectToOurs
        cat.CommonStateAllItems.value_or(HookState::RedirectToOurs) == HookState::RedirectToOurs
            ? HookState::RedirectToGTA
            : HookState::RedirectToOurs,
        [&] (HookState s) { NOTSA_LOG_ERR("TODO"); }, // cat.SetAllItemsState(s);
        [&] () { NOTSA_LOG_ERR("TODO"); } // cat.SetAlStepItemsToPreviousState();
    );
    if (!open) {
        return true;
    }
    //if (stateChanged) {
    //    cat.ToggleAlStepItemsState();
    //}
    //if (bool state = cbState; HandleSlideSetterForItem(state)) {
    //    cat.SetAlStepItemsState(state);
    //}
    //cat.Open(open);
    
    //if (!cat.Open()) {
    //    return;
    //}

    //
    // Draw hooks, and subcategories
    //

    // Draw hooks (items) (if any)
    if (hasItemsToShow) {
        if (hasCategoriesToShow) { // Render a separate tree node that's like a category for the items
            if (m_FilterProcessor.DidJustFinish) {
                if (cat.MaxFilterScoreOwnItems > 0.f) {
                    SetNextItemOpen(true, ImGuiCond_Always);
                }
            }
            NOTSA_LOG_ERR("TODO");
            //const auto [open, stateChanged] = TreeNodeWithCheckbox(
            //    "Hooks",
            //    cat.AreItemsLocked(),
            //    cat.m_AnyOurItemsUnhooked,
            //    cat.ItemsState(),
            //    cat.ItemsState().value_or(cat.m_LastSetOurState.value_or(HookState::RedirectToOurs)) == HookState::RedirectToOurs
            //        ? HookState::RedirectToGTA
            //        : HookState::RedirectToOurs,
            //    [&](HookState s) { cat.SetOurItemsState(s); },
            //    [&] () { cat.SetOurItemsToPreviousState(); }
            //);
            //
            //if (stateChanged) {
            //    //cat.SetOurItemsState(cbState);
            //    //cat.ToggleAlStepItemsState();
            //    NOTSA_UNREACHABLE("todo");
            //}
            //
            //if (open) {
            //    RenderCategoryItems(cat);
            //    TreePop();
            //}
        } else { // If there are no subcategories we can draw all items directly under this node
            RenderCategoryItems(cat);
        }
    }
    

    // Draw subcategories
    if (hasCategoriesToShow) {
        for (auto& v : cat.Categories) {
            RenderCategory(v);
        }
    }

    TreePop();

    return true;
}

const char* RHDebugModule::HooksDebugModule::GetWindowTitle() noexcept {
    m_WindowTitle.clear();
    const auto Append = [&](std::string_view fmt, auto&&... args) {
        std::vformat_to(std::back_inserter(m_WindowTitle), fmt, std::make_format_args(args...));
    };
    Append("ReversibleHooks (TM) (R)");
    std::unique_lock lock{ m_FilterProcessor.Mtx, std::try_to_lock };
    if (lock.owns_lock()) {
        Append(" - [Filtering: {} ms]", std::chrono::duration_cast<std::chrono::milliseconds>(m_FilterProcessor.TimeToFinish));
    } else {
        Append(" - [Status: Filtering...]");
    }
    Append("###ReversibleHooks"); // Keeps ID the same, as window title is used for it otherwise
    return m_WindowTitle.c_str();
}

RHDebugModule::HooksDebugModule::HooksDebugModule() :
    m_FilterProcessor{ .Thread{ [this] { FilteringThread(); } } }
{}

RHDebugModule::HooksDebugModule::~HooksDebugModule() {
    m_FilterProcessor.Exiting = true;
    m_FilterProcessor.CV.notify_one();
    m_FilterProcessor.Thread.join();
}

void HooksDebugModule::RenderWindow() {
    const notsa::ui::ScopedWindow window{ GetWindowTitle(), {500.f, 700.f}, m_IsOpen, ImGuiWindowFlags_MenuBar};
    if (!m_IsOpen) {
        return;
    }

    if (!m_ToRender) {
        m_ToRender = m_Builder.ConstructList(ReversibleHooks::RHManager::GetInstance().GetRootCategory());
    }

    UpdateSlideSetterMode();

    RenderMenuBar();
    RenderFilter();
    {
        std::unique_lock lock{ m_FilterProcessor.Mtx, std::try_to_lock };
        if (lock.owns_lock()) {
            if (!RenderCategory(*m_ToRender)) {
                notsa::ui::WindowCenteredTextUnformatted("No filter results");
            }
            m_FilterProcessor.DidJustFinish = false;
        } else {
            notsa::ui::WindowCenteredTextUnformatted("Filtering in progress...");
        }
    }
}

void HooksDebugModule::RenderMenuEntry() {
    notsa::ui::DoNestedMenuIL({ "Settings" }, [&] {
        ImGui::MenuItem("Hooks", nullptr, &m_IsOpen);
    });
}
