#pragma once

#include <thread>
#include <string>
#include <mutex>
#include <condition_variable>

#include <reversiblehooks/HookCategory.h>

#include "HookStepsDefs.h"
#include "HooksBuildListStep.h"
#include "HooksFilterListStep.h"

namespace RHDebugModule {
struct HooksListSteps {
    HooksBuildListStep  BuildStep;
    HooksFilterListStep FilterStep;

    StepsCategory*    Result{ nullptr };
};
using HooksListStepsPtr = std::unique_ptr<HooksListSteps>;

class HooksProcessingThread {
public:
    HooksProcessingThread();
    ~HooksProcessingThread();

    void PushSteps(HooksListStepsPtr steps) noexcept;
    HooksListStepsPtr GetResult();

    void ProcessingThreadLoop() noexcept;

private:
    std::thread             m_ProcessingThread; //!< Thread processing the list, filtering, etc
    HooksListStepsPtr       m_StepsToProcess;
    HooksListStepsPtr       m_ProcessedSteps;
    std::mutex              m_Mtx{};               //!< Locked as long as it's processing the list, unlocked when it's 
    bool                    m_ShouldExit{ false }; //!< If true, the thread should exit
};
}; // namespace RHDebugModule
