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

    static constexpr auto FILTER_INPUT_DEBOUNCE_TIME = std::chrono::milliseconds{ 50 };

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
    void OnDeserialized() override final { RunFilter(); }

    NOTSA_IMPLEMENT_DEBUG_MODULE_SERIALIZATION(HooksDebugModule, m_IsOpen, m_Filter);

private:
    bool HandleSlideSetterForItem(bool& inOutState); // Returns if state changed
    void UpdateSlideSetterMode();
    void FilteringThread();
    bool RunFilter();

    const char* GetWindowTitle() noexcept;

    void RenderFilter();
    void RenderMenuBar();
    bool RenderCategoryItems(StepsCategory& cat);
    bool RenderCategory(StepsCategory& cat);

private:
    bool m_IsOpen{};

    struct {
        FilterClock::duration     TimeToFinish{};
        std::thread               Thread;
        std::mutex                Mtx{};
        std::condition_variable   CV{};
        bool                      Exiting{};
        HookFilter                HookFilter{};
        bool                      DidJustFinish{};
    } m_FilterProcessor{};

    struct FilterOptions {
        std::optional<FilterClock::time_point> RunAt{};
        std::string                            Input{};
        HookFilter::Cutoffs                    Cutoffs{};
        bool                                   ShowScores{};
        bool                                   CaseSensitive{};

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(FilterOptions, Input, Cutoffs, ShowScores, CaseSensitive);
    } m_Filter;

    struct {
        SlideSetterMode Mode;
    } m_SlideSetter{};

    StepsCategory*     m_ToRender{}; //!< Data owned by `m_Builder`. Accessed/modified by the filtering thread, so must be protected by `m_FilterProcessor.Mtx` when accessed from the main thread.
    HooksBuildListStep m_Builder{};
    std::string        m_WindowTitle{};
};
}; // namespace RHDebugModule
using HooksDebugModule = RHDebugModule::HooksDebugModule;
