#include "StdInc.h"

#include "HooksProcessingThread.h"
#if 0
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

// instead of pointers use indices into the pool
// copy pools as-is when filtering
// keep build scancode on each category/item
// const name
// atomic<state>
// filtering just updates the scores using atomic<float> (pre-filtering we set all scores to 0.f on the main thread)
// when rendering hide everything with total score == 0.f
// sorting can be done by having 2 static_vector's using indices into the pool
// one `SortedXXX` and the other `AllXXX`, eg `SortedItems` and `AllItems`, along with an atomic bool `IsSorted` indicating which vector is safe to consume on the render thread
// if changes are detected on the main thread (user or code changes hook state) we re-calculate state in-place
// (We also maybe?? store a `LastSeenState` - updated each frame to check if code has changed the state)
// 

void HooksProcessingThread::ProcessingThreadLoop() noexcept {
#ifdef TRACY_ENABLE
    tracy::SetThreadName("RHDebugModule::HooksProcessingThread");
#endif
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
        ZoneScoped;

        const auto built = steps->BuildStep.Process();
        const auto filtered = steps->FilterStep.Process(built);
        steps->Result = filtered;

        lock.lock();
        m_ProcessedSteps = std::move(steps);
    }
}
}; // namespace RHDebugModule
#endif
