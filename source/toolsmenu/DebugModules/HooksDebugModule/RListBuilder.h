#pragma once

#include <reversiblehooks/HookCategory.h>
#include <Pool.h>

#include "RListDefs.h"

namespace RHDebugModule {
class RListBuilder {
public:
    RListBuilder(
        size_t maxItems      = 16'384,
        size_t maxCategories = 4'096
    );

    /*!
     * @brief Construct render list starting at the given `cat` (Should be the `RootCategory`)
     * @return Pointer to the first cateogry, owned by this class
     */
    RListCategory* ConstructList(std::shared_ptr<ReversibleHooks::HookCategory> cat) noexcept;

    /*!
     * @brief Re-calculate the common state of the category and its items
     * @param cat 
     * @return true if any of the common states or flags have changed, false otherwise
     */
    bool UpdateCategory(RListCategory& cat) const noexcept;

private:
    CPool<RListCategoryItem> m_PoolItem;
    CPool<RListCategory>     m_PoolCategory;
};
}; // namespace RHDebugModule
