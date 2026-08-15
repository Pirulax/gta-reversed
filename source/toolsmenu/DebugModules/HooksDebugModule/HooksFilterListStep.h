#pragma once

#include <optional>

#include "HookFilter.h"
#include "HookStepsDefs.h"

namespace RHDebugModule {
class HooksFilterListStep {
public:
    HooksFilterListStep(HookFilter filter) :
        m_Filter{ std::move(filter) } {
    }

    void Process(StepsCategory& root) const noexcept;

private:
    float CalculateCategoryScoresByName(StepsCategory& cat) const noexcept;

    float CalculateCategoryScoresByPath(StepsCategory& cat) const noexcept;
    float CalculateCategoryScoresByPath(StepsCategory& cat, const StepsCategory* parent, HookFilter::NamespaceTokens& ns, size_t depth) const noexcept;
    void  SetCategoryUnfilteredCategoryScores(StepsCategory& cat) const noexcept;

    std::optional<float> CalculateCategoryItemsScore(StepsCategory& cat, bool onlyIfCategoryHasScore) const noexcept;
    void                 SetCategoryUnfilteredItemsScore(StepsCategory& from) const noexcept;

    void CalculateCategoryMaxScores(StepsCategory& cat) const noexcept;

private:
    HookFilter m_Filter;
};
}; // namespace RHDebugModule
