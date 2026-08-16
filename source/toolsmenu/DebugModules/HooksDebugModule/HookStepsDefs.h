#pragma once

#include <optional>

#include <reversiblehooks/HookCategory.h>
#include <reversiblehooks/HookCategoryItem.h>

#include <ListItem_c.h>
#include <List_c.h>

namespace RHDebugModule {
using HookState     = ReversibleHooks::HookCategoryItem::HookState;
using CommonState   = std::optional<HookState>;

inline bool IsMatchingScoreOrNone(const std::optional<float>& score, float cutoff = 0.f) {
    return !score.has_value() || *score > cutoff;
}

struct StepsItem : ListItem_c<StepsItem> {
    std::optional<float>                               FilterScore{};
    std::shared_ptr<ReversibleHooks::HookCategoryItem> Ptr{};
};

struct StepsCategory : ListItem_c<StepsCategory> {
    std::shared_ptr<ReversibleHooks::HookCategory> Category{};
    std::optional<float>                           FilterScore{};            //!< Filter score of this category
    std::optional<float>                           MaxFilterScoreSubCats{};  //!< Only score of categories, not including their items
    std::optional<float>                           MaxFilterScoreOwnItems{}; //!< Max score of our items
    std::optional<float>                           MaxFilterScoreSubItems{}; //!< Max score of sub items
    std::optional<float>                           MaxScoreAllItems{};          //!< Max score of our items and sub items
    std::optional<float>                           MaxFilterScore{};         //!< max of all of the above
    bool                                           AnyUnhookedItems{};
    bool                                           AnyUnhookedOwnItems{};
    bool                                           AnyUnlockedItems{};
    bool                                           AnyUnlockedOwnItems{};
    std::optional<HookState>                       LastSetAllItemsState{};
    std::optional<HookState>                       LastSetAllItemState{};
    CommonState                                    CommonStateAllItems{}; //!< `nullopt` if there's no common state (There should be no categories with no items/sub-categories in this list)
    CommonState                                    CommonStateOwnItems{}; //!< `nullopt` if there's no common state, or no items
    CommonState                                    CommonStateSubItems{}; //!< `nullopt` if there's no common state, or no sub-categories
    TList_c<StepsItem>                             Items{};
    TList_c<StepsCategory>                         Categories{};
    size_t                                         NumCategoriesIgnored{}; //!< Number of categories ignored when then list was constructed because they were empty (no items and no sub-categories)
};
};
