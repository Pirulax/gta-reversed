#pragma once

#include "HookFilter.h"
#include "HookStepsDefs.h"

namespace RHDebugModule {
class HooksFilterListStep {
public:
    HooksFilterListStep(std::string_view filter) :
        m_Filter{ filter } {
    }

    StepsCategory* Process(StepsCategory* list) const noexcept;

private:
    /*!
     * @brief
     */
    bool ProcessCategorySimpleGlobalFilter(StepsCategory& cat) const noexcept;

    /*!
     * @brief Filters category's items and subcategories in-place
     * @returns If the category has any items or subcategories that match the filter
     */
    auto ProcessCategory(StepsCategory& from, HookFilter::NamespaceTokens& nmspace, size_t depth = 0) const noexcept;

    bool ProcessCategoryHookFilterOnly(StepsCategory& from) const noexcept;

    /*!
     * @brief Filters category's items in-place
     * @returns If the category has any items that match the filter
     */
    bool ProcessItems(StepsCategory& cat, bool noFilter = false) const noexcept;

private:
    HookFilter m_Filter;
};
}; // namespace RHDebugModule
