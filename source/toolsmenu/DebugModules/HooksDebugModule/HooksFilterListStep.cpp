#include "StdInc.h"

#include "HooksFilterListStep.h"

namespace RHDebugModule {
void HooksFilterListStep::Process(StepsCategory& root) const noexcept {
    ZoneScoped;

    // NB: Always run category filtering first, because item filtering depends on category scores to determine if it should filter items or not
    if (m_Filter.IsFilteringByCategoryPath()) {
        CalculateCategoryScoresByPath(root);
    } else if (m_Filter.IsFilteringByCategoryName()) {
        CalculateCategoryScoresByName(root);
    } else {
        SetCategoryUnfilteredCategoryScores(root);
    }

    // NB: Always run item filtering after category (See above why)
    if (m_Filter.IsHookFilterActive()) {
        CalculateCategoryItemsScore(root, m_Filter.IsFilteringByCategory() && !m_Filter.IsSimpleFilterString());
    } else {
        SetCategoryUnfilteredItemsScore(root);
    }

    // After all this we can calculate the total scores
    CalculateCategoryMaxScores(root);
}

float HooksFilterListStep::CalculateCategoryScoresByName(StepsCategory& cat) const noexcept {
    cat.FilterScore           = m_Filter.MatchCategoryByName(cat.Category->Name());
    cat.MaxFilterScoreSubCats = std::nullopt;
    for (auto& sc : cat.Categories) {
        cat.MaxFilterScoreSubCats = std::max(cat.MaxFilterScoreSubCats.value_or(0.f), CalculateCategoryScoresByName(sc));
    }
    return std::max(cat.MaxFilterScoreSubCats.value_or(0.f), *cat.FilterScore);
}

float HooksFilterListStep::CalculateCategoryScoresByPath(StepsCategory& cat, HookFilter::NamespaceTokens& ns, size_t depth) const noexcept {
    cat.FilterScore           = m_Filter.MatchCategoryByNamespace(ns, depth);
    cat.MaxFilterScoreSubCats = std::nullopt;

    if (m_Filter.IsRootRelativeNamespace()) {
        if (cat.FilterScore <= 0.f) {
            return 0.f; // If we didn't match neither will any of our sub-categories, so we can just stop here
        }
    }

    for (auto& sc : cat.Categories) {
        ns.push_back(sc.Category->Name());
        cat.MaxFilterScoreSubCats = std::max(cat.MaxFilterScoreSubCats.value_or(0.f), CalculateCategoryScoresByPath(sc, ns, depth + 1));
        ns.pop_back();
    }

    return std::max(cat.MaxFilterScoreSubCats.value_or(0.f), *cat.FilterScore);
}

float HooksFilterListStep::CalculateCategoryScoresByPath(StepsCategory& cat) const noexcept {
    HookFilter::NamespaceTokens ns = { cat.Category->Name() };
    return CalculateCategoryScoresByPath(cat, ns, 0);
}

void HooksFilterListStep::SetCategoryUnfilteredCategoryScores(StepsCategory& cat) const noexcept {
    cat.FilterScore           = std::nullopt;
    cat.MaxFilterScoreSubCats = std::nullopt;
    for (auto& sc : cat.Categories) {
        SetCategoryUnfilteredCategoryScores(sc);
    }
}

std::optional<float> HooksFilterListStep::CalculateCategoryItemsScore(StepsCategory& cat, bool onlyIfCategoryHasScore) const noexcept {
    // NB: For this to work properly category scores must first be calculated!
    if (onlyIfCategoryHasScore != (std::max(cat.MaxFilterScoreSubCats, cat.FilterScore) > 0.f)) {
        SetCategoryUnfilteredItemsScore(cat);
        return std::nullopt;
    }

    if (cat.HasItems) {
        cat.MaxFilterScoreOwnItems = 0.f;
        for (auto& i : cat.Items) {
            const auto score = m_Filter.MatchItem(
                i.Ptr->GetName(),
                i.Ptr->GetHookAddressGTA(),
                i.Ptr->GetHookAddressOur()
            );
            i.FilterScore = score;
            cat.MaxFilterScoreOwnItems = std::max(*cat.MaxFilterScoreOwnItems, score);
        }
    } else {
        cat.MaxFilterScoreOwnItems = std::nullopt;
    }

    cat.MaxFilterScoreSubItems = std::nullopt;
    for (auto& sc : cat.Categories) {
        cat.MaxFilterScoreSubItems = std::max(cat.MaxFilterScoreSubItems, CalculateCategoryItemsScore(sc, onlyIfCategoryHasScore));
    }

    cat.MaxScoreAllItems = std::max(cat.MaxFilterScoreSubItems, cat.MaxFilterScoreOwnItems);

    return cat.MaxScoreAllItems;
}

void HooksFilterListStep::SetCategoryUnfilteredItemsScore(StepsCategory& cat) const noexcept {
    cat.MaxFilterScoreOwnItems = std::nullopt;
    cat.MaxFilterScoreSubItems = std::nullopt;
    cat.MaxScoreAllItems       = std::nullopt;
    for (auto& i : cat.Items) {
        i.FilterScore = std::nullopt;
    }
    for (auto& sc : cat.Categories) {
        SetCategoryUnfilteredItemsScore(sc);
    }
}
void HooksFilterListStep::CalculateCategoryMaxScores(StepsCategory& cat) const noexcept {
    cat.MaxFilterScore = std::max(cat.MaxFilterScoreSubCats, cat.MaxScoreAllItems);
    for (auto& sc : cat.Categories) {
        CalculateCategoryMaxScores(sc);
    }
}
}; // namespace RHDebugModule
