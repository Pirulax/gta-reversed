#pragma once


#include <thread>
#include <optional>
#include <string>
#include <string_view>

#include <boost/static_string.hpp>

#include <reversiblehooks/HookCategory.h>
#include <reversiblehooks/HookCategoryItem.h>

#include <Pool.h>
#include <ListItem_c.h>
#include <List_c.h>

#include "HookStepsDefs.h"

namespace RHDebugModule {
class HooksBuildListStep {
public:
    HooksBuildListStep(
        std::shared_ptr<ReversibleHooks::HookCategory> cat,
        size_t                                         maxItems      = 16'384,
        size_t                                         maxCategories = 4096
    );

    StepsCategory* Process() { return m_Result; } // We've already built the list on the calling thread, because of thread safety
    StepsCategory* ConstructList(std::shared_ptr<ReversibleHooks::HookCategory> cat) noexcept;
    StepsCategory* GetResult() const noexcept { return m_Result; }

private:
    StepsCategory*       m_Result;
    CPool<StepsItem>     m_PoolItem;
    CPool<StepsCategory> m_PooStepCategory;
};
}; // namespace RHDebugModule
