#include "StdInc.h"

#include <string>
#include <chrono>
#include <optional>
#include <format>

#include <imgui.h>
#include <libs/imgui/misc/cpp/imgui_stdlib.h>

#include <TristateCheckbox.h>

#include <extensions/CustomFormatters.hpp>
#include <reversiblehooks/ReversibleHooks.h>
#include <reversiblehooks/HookCategory.h>

#include "Utility.h"
#include "HooksDebugModule.h"

#include "HooksSortListStep.hpp"

namespace RH = ReversibleHooks;
namespace rng = std::ranges;

using namespace RHDebugModule;
using namespace ImGui;

using HookState = ReversibleHooks::ReversibleHook::TwoWayHookState;

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
        if (!m_HooksList.RootCategory) {
            continue; // Nothing to filter
        }
        /* Hold lock until we finish */
        NOTSA_LOG_DEBUG("Running filter");
        const auto now = FilterClock::now();
        HooksFilterListStep{ std::move(m_FilterProcessor.HookFilter) }.Process(*m_HooksList.RootCategory);
        HooksSortListStep{}.Process(*m_HooksList.RootCategory);
        m_FilterProcessor.FinishedAt        = FilterClock::now();
        m_FilterProcessor.NeedToAckFinished = true;
    }
}

bool RHDebugModule::HooksDebugModule::RunFilter() {
    if (!m_HooksList.RootCategory) {
        return false; // Nothing to filter
    }
    {
        std::unique_lock lock{ m_FilterProcessor.Mtx, std::try_to_lock };
        if (!lock.owns_lock()) {
            return false; // Filter still running
        }
        m_FilterProcessor.HookFilter = { m_Filter.Input, m_Filter.CaseSensitive, m_Filter.Cutoffs };
        m_FilterProcessor.StartedAt  = FilterClock::now();
    }
    m_FilterProcessor.CV.notify_one();
    return true;
}

void RHDebugModule::HooksDebugModule::CheckNeedsToRunFilter() {
    if (std::exchange(m_Filter.Changed, false)) {
        m_Filter.RunAt = FilterClock::now() + FILTER_INPUT_DEBOUNCE_TIME;
    } else if (m_Filter.RunAt.has_value() && *m_Filter.RunAt < FilterClock::now()) {
        if (RunFilter()) {
            m_Filter.RunAt = std::nullopt;
        }
    }
}

void RHDebugModule::HooksDebugModule::RenderFilter() {
    SetNextItemWidth(-1.f);
    m_Filter.Changed |= InputText("##FilterInput", &m_Filter.Input);
    SetItemTooltip(
        "`cpy`                - Filter by category/function\n"
        "`cped::`             - Filter by category name\n"
        "`ped/player`         - Filter by category path\n"
        "`::function`         - Filter by function name, any category\n"
        "`ped/player::busted` - Filter by both category path and function name\n"
        "`/entity`            - Filter relative to root, must match whole path\n"
        "Use `*` as wildrcard\n"
        "For more tips see gta-reversed-modern/discussions/190\n"
    );
}

const char* StateToString(HookState state) {
    switch (state) {
    case HookState::Unhooked:       return "unhooked";
    case HookState::RedirectToGTA:  return "gta";
    case HookState::RedirectToOurs: return "our";
    default:                        return "unk";
    }
}

template<
    std::predicate<HookState> SetStateFn,
    std::predicate<>          RestoreStateFn>
bool StateButton(
    const char*              title,
    bool                     disabled,
    ImTristate               onOffCheckboxState,
    std::optional<HookState> current,
    HookState                next,
    SetStateFn&&             SetState,
    RestoreStateFn&&         RestoreState
) {
    notsa::ui::ScopedID      idg{ "state" };
    notsa::ui::ScopedDisable sdg{ disabled };

    bool changed = false;

    SameLine(); 
    if (bool checked; CheckboxTristate("##on-off", onOffCheckboxState, checked)) {
        if (checked) {
            changed |= RestoreState();
        } else {
            changed |= SetState(HookState::Unhooked);
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
        changed |= SetState(next);
    }
    PopStyleColor();

    SameLine();
    TextUnformatted(title);

    return changed;
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
        if (ImGui::BeginMenu("Options")) {
            if (ImGui::BeginMenu("Filter")) {
                m_Filter.Changed |= Checkbox("Show Filter Scores", &m_Filter.ShowScores);
                m_Filter.Changed |= Checkbox("Case Sensitive", &m_Filter.CaseSensitive);
                if (BeginMenu("Cutoffs")) {
                    const auto CutoffSlider = [&](float* value, const char* name) {
                        m_Filter.Changed |= SliderFloat(name, value, 0.f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                    };
                    CutoffSlider(&m_Filter.Cutoffs.CategoryInPath, "CategoryInPath");
                    CutoffSlider(&m_Filter.Cutoffs.CategoryGlobal, "CategoryGlobal");
                    CutoffSlider(&m_Filter.Cutoffs.ItemGlobal, "ItemGlobal");
                    CutoffSlider(&m_Filter.Cutoffs.ItemLocal, "ItemLocal");
                    CutoffSlider(&m_Filter.Cutoffs.ItemAddress, "ItemAddress");
                    if (Button("Reset Cutoffs")) {
                        m_Filter.Cutoffs = {};
                        m_Filter.Changed = true;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

bool HooksDebugModule::RenderCategoryItems(StepsCategory& cat) {
    if (!IsMatchingScoreOrNone(cat.MaxFilterScoreOwnItems)) {
        return false;
    }
    bool changed = false;
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
        changed |= StateButton(
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
            [&] (HookState s) { return item.Ptr->SetState(s); },
            [&] () { return item.Ptr->SetToPreviousState(); }
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
    return changed;
}


auto HooksDebugModule::RenderCategory(StepsCategory& cat) -> RenderCategoryResult {
    if (!cat.Items.IsEmpty() && !cat.Categories.IsEmpty()) {
        return RenderCategoryResult::SKIPPED_NOTHING_TO_SHOW;
    }

    if (!IsMatchingScoreOrNone(cat.MaxFilterScore)) {
        return RenderCategoryResult::SKIPPED_FILTERED;
    }

    const auto hasOwnItemsToShow = !cat.Items.IsEmpty() && IsMatchingScoreOrNone(cat.MaxFilterScoreOwnItems),
               hasSubItemsToShow = !cat.Categories.IsEmpty() && IsMatchingScoreOrNone(cat.MaxFilterScoreSubItems);

    if (!hasOwnItemsToShow && !hasSubItemsToShow) {
        return RenderCategoryResult::SKIPPED_FILTERED;
    }

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
        const auto changed = StateButton(
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

        return std::make_tuple(open, changed);
    };
    
    const auto SetCategoryOwnItemsState = [&](StepsCategory& c, HookState state) {
        return rng::fold_left(c.Items, false, [state](bool changed, StepsItem& item) {
            return item.Ptr->SetState(state) || changed;
        });
    };

    const auto RestoreCategoryOwnItemsState = [&](StepsCategory& c) {
        return rng::fold_left(c.Items, false, [](bool changed, StepsItem& item) {
            return changed | item.Ptr->SetToPreviousState();
        });
    };

    //
    // Category tree node
    //
    if (m_FilterProcessor.NeedToAckFinished) {
        SetNextItemOpen(cat.MaxScoreAllItems > 0.f || cat.MaxFilterScoreSubCats > 0.f, ImGuiCond_Always);
    }

    const auto [open, categoryStateChanged] = TreeNodeWithCheckbox(
        m_Filter.ShowScores
            ? std::format(
                  "{} [Max filter scores: {{Own: {:.2f}, OwnItems: {:.2f}, SubItems: {:.2f}, AllItems: {:.2f}, SubCats: {:.2f}, All: {:.2f}}}",
                  cat.Category->Name(),
                  cat.FilterScore,
                  cat.MaxFilterScoreOwnItems,
                  cat.MaxFilterScoreSubItems,
                  cat.MaxScoreAllItems,
                  cat.MaxFilterScoreSubCats,
                  cat.MaxFilterScore
              ).c_str()
            : cat.Category->Name().c_str(),
        !cat.AnyUnlockedItems,
        cat.AnyUnhookedItems,
        cat.CommonStateAllItems,
        cat.CommonStateAllItems.value_or(cat.LastSetOurItemsState.value_or(HookState::RedirectToOurs)) == HookState::RedirectToOurs
            ? HookState::RedirectToGTA
            : HookState::RedirectToOurs,
        [&] (HookState state) -> bool {
            cat.LastSetOurItemsState = state;
            return [&, state](this auto&& Self, StepsCategory& c) -> bool { // Set state of all items and sub-categories using a recursive lambda
                bool changed = SetCategoryOwnItemsState(c, state);
                for (auto& sc : c.Categories) {
                    changed |= Self(sc);
                }
                return changed;
            }(cat);
        },
        [&] () -> bool {
            return [&](this auto&& Self, StepsCategory& c) -> bool { // Restore state of all items and sub-categories using a recursive lambda
                bool changed = RestoreCategoryOwnItemsState(c);
                for (auto& sc : c.Categories) {
                    changed |= Self(sc);
                }
                return changed;
            }(cat);
        }
    );
    if (!open) {
        return RenderCategoryResult::SKIPPED_CLOSED;
    }

    const auto hasSubCategoriesToShow = !cat.Categories.IsEmpty() && (IsMatchingScoreOrNone(cat.MaxFilterScoreSubCats) || IsMatchingScoreOrNone(cat.MaxFilterScoreSubItems));

    // Draw items (hooks) (if any)
    bool itemsStateChanged = false;
    if (hasOwnItemsToShow) {
        if (hasSubItemsToShow) { // Render a separate tree node that's like a category for the items
            if (m_FilterProcessor.NeedToAckFinished) {
                if (cat.MaxFilterScoreOwnItems > 0.f) {
                    SetNextItemOpen(cat.MaxFilterScoreOwnItems > 0.f, ImGuiCond_Always);
                }
            }
            const auto [open, stateChanged] = TreeNodeWithCheckbox(
                m_Filter.ShowScores
                    ? std::format("Hooks [Max filter score: {}]", cat.MaxFilterScoreOwnItems).c_str()
                    : "Hooks",
                !cat.AnyUnlockedOurItems,
                cat.AnyUnhookedOurItems,
                cat.CommonStateOwnItems,
                cat.CommonStateOwnItems.value_or(cat.LastSetOurItemsState.value_or(HookState::RedirectToOurs)) == HookState::RedirectToOurs
                    ? HookState::RedirectToGTA
                    : HookState::RedirectToOurs,
                [&] (HookState s) -> bool {
                    cat.LastSetOurItemsState = s;
                    return SetCategoryOwnItemsState(cat, s);
                },
                [&] () -> bool {
                    return RestoreCategoryOwnItemsState(cat);
                }
            );
            if (open) {
                RenderCategoryItems(cat);
                TreePop();
            }
        } else { // If there are no subcategories we can draw all items directly under this node
            itemsStateChanged |= RenderCategoryItems(cat);
        }
    }

    // Draw subcategories
    bool subCategoriesStateChanged = false;
    if (hasSubItemsToShow) {
        for (auto& v : cat.Categories) {
            subCategoriesStateChanged |= RenderCategory(v) == RenderCategoryResult::RENDERED_CATEGORY_STATE_CHANGED;
        }
    }

    // Pop the category's tree node
    TreePop();

    // Now check if we've changed, and if so, check if that change affects the state of the parent
    bool changed = itemsStateChanged || categoryStateChanged || subCategoriesStateChanged;
    if (changed) {
        changed &= m_HooksList.Builder.UpdateCategory(cat);
    }

    return changed
        ? RenderCategoryResult::RENDERED_CATEGORY_STATE_CHANGED
        : RenderCategoryResult::RENDERED;
}

const char* RHDebugModule::HooksDebugModule::GetWindowTitle() noexcept {
    m_WindowTitle.clear();
    const auto Append = [&](std::string_view fmt, auto&&... args) {
        std::vformat_to(std::back_inserter(m_WindowTitle), fmt, std::make_format_args(args...));
    };
    Append("ReversibleHooks (TM) (R)");
    std::unique_lock lock{ m_FilterProcessor.Mtx, std::try_to_lock };
    if (lock.owns_lock()) {
        Append(" - [Filtering: {} ms]", std::chrono::duration_cast<std::chrono::milliseconds>(m_FilterProcessor.FinishedAt - m_FilterProcessor.StartedAt));
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

    if (!m_HooksList.RootCategory) {
        m_HooksList.RootCategory = m_HooksList.Builder.ConstructList(ReversibleHooks::RHManager::GetInstance().GetRootCategory());
    }

    UpdateSlideSetterMode();

    RenderMenuBar();
    RenderFilter();
    {
        std::unique_lock lock{ m_FilterProcessor.Mtx, std::try_to_lock };
        if (lock.owns_lock()) {
            switch (RenderCategory(*m_HooksList.RootCategory)) {
            case RenderCategoryResult::RENDERED_CATEGORY_STATE_CHANGED: {
                m_HooksList.Builder.UpdateCategory(*m_HooksList.RootCategory);
                break;
            }
            case RenderCategoryResult::SKIPPED_FILTERED: {
                notsa::ui::WindowCenteredTextUnformatted("No filter results");
                break;
            }
            }
            m_FilterProcessor.NeedToAckFinished = false;
        } else {
            notsa::ui::WindowCenteredTextUnformatted("Filtering in progress...");
        }
    }

    CheckNeedsToRunFilter();
}

void HooksDebugModule::RenderMenuEntry() {
    notsa::ui::DoNestedMenuIL({ "Settings" }, [&] {
        ImGui::MenuItem("Hooks", nullptr, &m_IsOpen);
    });
}
