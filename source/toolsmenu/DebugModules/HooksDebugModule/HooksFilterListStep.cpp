#include "StdInc.h"

#include "HooksFilterListStep.h"

namespace RHDebugModule {
//auto HooksFilterListStep::ProcessCategory(
//    StepsCategory& cat,
//    const HookFilter&              filter,
//    NamespaceTokens&                 path,
//    size_t                         depth
//) noexcept {
//    // Update current path
//    path.emplace_back(cat.Name);
//    const notsa::ScopeGuard pg{ [&path]() {
//        path.pop_back();
//    } };
//
//    // Check if current category matches filter
//    cat.FilterScore           = m_Filter.MatchCategoryByNamespace(cat.Name, depth);
//    const auto isThisMatching = cat.FilterScore.has_value();
//
//    const auto ProcessSubCategories = [&, this]() {
//        auto cats = std::move(cat.SubCategories);
//        for (auto& sc : cats) {
//            if (ProcessCategory(sc, filter, path, depth + 1, isThisMatching)) {
//                cat.SubCategories.AddItem(&sc);
//            }
//        }
//        return !cat.SubCategories.IsEmpty();
//    };
//
//    // When combining results it's enough if:
//    // - Either we have items that match the hook filter
//    // - We have sub-categories that match the namespace filter or have items that match the hook filter
//    if (m_Filter.ShouldCombineFilterResults() && m_Filter.IsHookFilterActive() && m_Filter.IsNamespaceFilterActive()) {
//        const auto hasMatchingItems         = ProcessItems(cat, filter);
//        const auto hasMatchingSubCategories = ProcessSubCategories();
//        return hasMatchingSubCategories || isThisMatching && hasMatchingItems;
//    }
//
//    // In root-relative namespace mode the current path has to match up all the way, if not, we stop
//    if (m_Filter.IsRootRelativeNamespace() && !isThisMatching) {
//        return false;
//    }
//
//    // Otherwise if the filter matches then we show the category
//    const auto hasMatchingItems         = ProcessItems(cat, filter) && m_Filter.IsHookFilterActive();
//    const auto hasMatchingSubCategories = ProcessSubCategories();
//    return hasMatchingItems || hasMatchingSubCategories || isThisMatching && isParentMatching;
//}

StepsCategory* HooksFilterListStep::Process(StepsCategory* list) const noexcept {
    if (m_Filter.IsSimpleGlobalSearch()) {
        ProcessCategorySimpleGlobalFilter(*list);
    } else if (m_Filter.IsHookFilterActive() && !m_Filter.IsNamespaceFilterActive()) {
        ProcessCategoryHookFilterOnly(*list);
    } else {
        NOTSA_LOG_ERR("TODO");
    }
    return list;
}

bool HooksFilterListStep::ProcessCategorySimpleGlobalFilter(StepsCategory& cat) const noexcept {
    // Match category by name
    cat.FilterScore = m_Filter.MatchCategoryByName(cat.Name);

    // Always process items, but only filter if the category itself doesn't match the filter (otherwise we want to show kep items)
    const auto isCategoryMatching = cat.FilterScore != 0.f;
    const auto hasItemsMatching   = ProcessItems(cat, isCategoryMatching);

    // Also process sub-categories, and calculate total score for them
    if (!cat.SubCategories.IsEmpty()) {
        cat.SubCategories.Filter([this](auto& sc) {
            return ProcessCategorySimpleGlobalFilter(sc);
        });
        cat.TotalFilterScore = rng::fold_left(cat.SubCategories, cat.FilterScore + cat.TotalFilterScoreOurItems, [](float sum, const auto& sc) {
            return sum + sc.TotalFilterScore;
        });
    }

    // If we have any items (including those of sub-categories) that match the filter, keep the category
    return isCategoryMatching || hasItemsMatching || !cat.SubCategories.IsEmpty();
}

bool HooksFilterListStep::ProcessCategoryHookFilterOnly(StepsCategory& of) const noexcept {
    const auto hasItemsMatching = ProcessItems(of);
    of.SubCategories.Filter([this](auto& c) {
        return ProcessCategoryHookFilterOnly(c);
    });
    return hasItemsMatching || !of.SubCategories.IsEmpty();
}

bool HooksFilterListStep::ProcessItems(StepsCategory& cat, bool noFilter) const noexcept {
    if (m_Filter.IsHookFilterActive()) {
        const auto DoMatch = [this](auto& i) {
            const auto score = i.FilterScore = m_Filter.MatchItem(
                i.Name.c_str(),
                i.Item->GetHookAddressGTA(),
                i.Item->GetHookAddressOur()
            );
            return score;
        };
        if (noFilter) {
            for (auto& i : cat.Items) {
                DoMatch(i);
            }
        } else {
            cat.Items.Filter([&DoMatch](auto& i) {
                return DoMatch(i) != 0.f;
            });
        }
        cat.TotalFilterScoreOurItems = rng::fold_left(cat.Items, 0.0f, [](float sum, const auto& i) {
            return sum + i.FilterScore;
        });
    }
    return !cat.Items.IsEmpty();
}
}; // namespace RHDebugModule
