#pragma once

#include "HookStepsDefs.h"

namespace RHDebugModule {
class HooksSortListStep {
public:
    HooksSortListStep();

    void Process(StepsCategory& cat) noexcept;

private:
    std::vector<StepsCategory*> m_BufSortedCategories;
    std::vector<StepsItem*>     m_BufSortedItems;
};
};
