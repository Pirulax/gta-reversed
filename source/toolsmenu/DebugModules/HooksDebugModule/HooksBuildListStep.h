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
        size_t                                         maxItems      = 16'384,
        size_t                                         maxCategories = 4096
    );

    StepsCategory* ConstructList(std::shared_ptr<ReversibleHooks::HookCategory> cat) noexcept;

private:
    CPool<StepsItem>     m_PoolItem;
    CPool<StepsCategory> m_PooStepCategory;
};
}; // namespace RHDebugModule
