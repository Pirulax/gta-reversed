#pragma once

#include <thread>
#include <chrono>
#include <bitset>
#include <string>

#include <toolsmenu/DebugModules/DebugModule.h>
#include <toolsmenu/TristateCheckbox.h>

#include "RListDefs.h"
#include "HookFilter.h"
#include "RListBuilder.h"

namespace ReversibleHooks {
class HookCategory;
};

namespace RHDebugModule {
class HooksDebugModule final : public DebugModule {
    using HookState   = ReversibleHooks::ReversibleHook::TwoWayHookState;
    using FilterClock = std::chrono::steady_clock;

public:
    HooksDebugModule();
    ~HooksDebugModule();

    void RenderWindow() override final;
    void RenderMenuEntry() override final;

    void OnDeserialized() override final { m_Filter.Changed = true; }

    NOTSA_IMPLEMENT_DEBUG_MODULE_SERIALIZATION(HooksDebugModule, m_IsOpen, m_Filter);

private:
    template<std::predicate<HookState> SetStateFn>
    bool HandleSlideSetterForItem(std::optional<HookState> state, HookState next, SetStateFn&& SetState);

    void UpdateSlideSetterMode();

    template<std::predicate<HookState> SetStateFn, std::predicate<> RestoreStateFn>
    bool StateChanger(
        const char*              title,
        bool                     disabled,
        ImGui::ImTristate        onOffCheckboxState,
        std::optional<HookState> current,
        HookState                next,
        SetStateFn&&             SetState,
        RestoreStateFn&&         RestoreState
    );

    void FilteringThread();
    bool RunFilter();
    void CheckNeedsToRunFilter();

    void RenderFilter();
    void RenderMenuBar();
    void RenderHooksSection();
    void RenderFooter();

    bool RenderCategoryItems(RListCategory& cat);
    enum class RenderCategoryResult {
        RENDERED,                        //!< Rendered, no changes
        RENDERED_CATEGORY_STATE_CHANGED, //!< Category was rendered, and it's state has changed (due to user interaction) in a way that affects the state of its parent category
        SKIPPED_NOTHING_TO_SHOW,         //!< No items/sub-categories to show
        SKIPPED_FILTERED,                //!< Category has items/sub-categories, but none of them match the filter
        SKIPPED_CLOSED,                  //!< Category's tree node was closed
    };
    RenderCategoryResult RenderCategory(RListCategory& cat);

private:
    bool m_IsOpen{};

    struct {
        FilterClock::time_point StartedAt{};
        FilterClock::time_point FinishedAt{};
        std::thread             Thread;
        std::mutex              Mtx{};
        std::condition_variable CV{};
        bool                    Exiting{};
        HookFilter              HookFilter{};
        bool                    NeedToAckFinished{};
        RListCategory*          ListToFilter{};
    } m_FilterProcessor{};

    struct FilterOptions {
        std::optional<FilterClock::time_point> RunAt{};
        std::string                            Input{};
        HookFilter::Cutoffs                    Cutoffs{};
        bool                                   ShowScores{};
        bool                                   CaseSensitive{};
        bool                                   Changed{};

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(FilterOptions, Input, Cutoffs, ShowScores, CaseSensitive);
    } m_Filter;

    struct SlideSetter {
        enum class Mode {
            NONE,
            PICK_THEN_SET,
            SET,
            TOGGLE
        } Mode{};
        HookState StateToSet;
        ImGuiID   LastUsedOnID{};
    } m_SlideSetter{};

    struct {
        RListBuilder   Builder;
        RListCategory* RootCategory; //!< Data is owned by the `Builder`
    } m_RenderList{};

    struct HooksExport {
        std::bitset<+HookState::Count> SelectedStates{};        //< States to export
        bool                           ExcludeSelectedStates{}; //!< Exclude (instead of including) the selected states
        bool                           OnlyFiltered{};          //!< Export only items that match the current filter (Or all, if no filter)
    } m_HooksExport;
};
}; // namespace RHDebugModule

using HooksDebugModule = RHDebugModule::HooksDebugModule;
