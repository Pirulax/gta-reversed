#pragma once

#include <thread>
#include <chrono>
#include <future>
#include <string>
#include <string_view>

#include <toolsmenu/DebugModules/DebugModule.h>

#include "HooksProcessingThread.h"

namespace ReversibleHooks {
class HookCategory;
};

namespace RHDebugModule {
class HooksDebugModule final : public DebugModule {
    using FilterClock = std::chrono::steady_clock;

    static constexpr auto FILTER_INPUT_DEBOUNCE_TIME = std::chrono::milliseconds{ 250 };

    enum class SlideSetterMode {
        NONE,
        SETTER, // This mode turns into either `TURN_OFF` OR `TURN_ON` as soon as it's possible
        TURN_ON,
        TURN_OFF,
        TOGGLE
    };

public:
    HooksDebugModule();
    ~HooksDebugModule();

    void RenderWindow() override final;
    void RenderMenuEntry() override final;
    void OnDeserialized() override final { m_Filter.Changed = true; }

    NOTSA_IMPLEMENT_DEBUG_MODULE_SERIALIZATION(HooksDebugModule, m_IsOpen, m_Filter);

private:
    bool HandleSlideSetterForItem(bool& inOutState); // Returns if state changed
    void UpdateSlideSetterMode();
    void FilteringThread();
    bool RunFilter();
    void CheckNeedsToRunFilter();

    const char* GetWindowTitle() noexcept;

    void RenderFilter();
    void RenderMenuBar();
    bool RenderCategoryItems(StepsCategory& cat);

    enum class RenderCategoryResult {
        RENDERED,                        //!< Rendered, no changes
        RENDERED_CATEGORY_STATE_CHANGED, //!< Category was rendered, and it's state has changed (due to user interaction) in a way that affects the state of its parent category
        SKIPPED_NOTHING_TO_SHOW,         //!< No items/sub-categories to show
        SKIPPED_FILTERED,                //!< Category has items/sub-categories, but none of them match the filter
        SKIPPED_CLOSED,                  //!< Category's tree node was closed
    };
    RenderCategoryResult RenderCategory(StepsCategory& cat);

private:
    bool m_IsOpen{};

    struct {
        FilterClock::time_point   StartedAt{};
        FilterClock::time_point   FinishedAt{};
        std::thread               Thread;
        std::mutex                Mtx{};
        std::condition_variable   CV{};
        bool                      Exiting{};
        HookFilter                HookFilter{};
        bool                      NeedToAckFinished{};
        StepsCategory*            ListToFilter{};
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

    struct {
        SlideSetterMode Mode{};
    } m_SlideSetter{};

    struct HookList {
        HooksBuildListStep Builder;
        StepsCategory*     RootCategory; //!< Data is owned by the `Builder`
    } m_HooksList{};

    std::string    m_WindowTitle{};
};
}; // namespace RHDebugModule
using HooksDebugModule = RHDebugModule::HooksDebugModule;
