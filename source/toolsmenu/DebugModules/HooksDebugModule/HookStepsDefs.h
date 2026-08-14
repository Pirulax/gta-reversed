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

namespace RHDebugModule {
using HookState     = ReversibleHooks::HookCategoryItem::HookState;
using CommonState   = std::optional<HookState>;
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
    bool                                           AnyUnlockedItems{};
    bool                                           HasItems{};
    bool                                           HasSubCategories{};
    CommonState                                    CommonStateAllItems{};
    CommonState                                    CommonStateOwnItems{};
    CommonState                                    CommonStateSubItems{};
    TList_c<StepsItem>                             Items{};
    TList_c<StepsCategory>                         Categories{};
};
};
