#include "StdInc.h"

#include "HooksProcessingThread.h"

namespace RHDebugModule {
HooksProcessingThread::HooksProcessingThread() :
    m_ProcessingThread{ [this] {
        ProcessingThreadLoop();
    } }
{
}

HooksProcessingThread::~HooksProcessingThread()  {
    {
        std::scoped_lock lock{ m_Mtx };
        m_ShouldExit = true;
    }
    if (m_ProcessingThread.joinable()) {
        m_ProcessingThread.join();
    }
}

void HooksProcessingThread::PushSteps(HooksListStepsPtr steps) noexcept {
    std::scoped_lock lock{ m_Mtx };
    m_StepsToProcess = std::move(steps);
}

HooksListStepsPtr HooksProcessingThread::GetResult() {
    std::scoped_lock lock{ m_Mtx };
    return m_ProcessedSteps;
}

void HooksProcessingThread::ProcessingThreadLoop() noexcept {
    while (!m_ShouldExit) {
        std::unique_lock lock{ m_Mtx };
        if (m_ShouldExit) {
            break;
        }
        if (!m_StepsToProcess) {
            continue;
        }

        auto steps = std::exchange(m_StepsToProcess, nullptr);
        lock.unlock();

        const auto built = steps->BuildStep.Process();
        const auto filtered = steps->FilterStep.Process(built);
        steps->Result = filtered;

        lock.lock();
        m_ProcessedSteps = std::move(steps);
    }
}
}; // namespace RHDebugModule
