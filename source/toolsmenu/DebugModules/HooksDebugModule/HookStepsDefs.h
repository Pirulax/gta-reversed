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
    boost::static_string<256>                          Name{};
    float                                              FilterScore{};
    std::shared_ptr<ReversibleHooks::HookCategoryItem> Item{}; // TODO: Make address pointers atomic so we can safely access them when filtering
};

struct StepsCategory : ListItem_c<StepsCategory> {
    boost::static_string<256> Name{};
    float                     FilterScore{};
    float                     TotalFilterScore{};         //!< Total score, including sub-categories and items
    float                     TotalFilterScoreOurItems{}; //!< Total score of (our) items only
    bool                      HasMatchingSubCategories{};
    bool                      AnyUnhookedItems{};
    bool                      AnyUnlockedItems{};
    bool                      HasItems{};
    bool                      HasSubCategories{};
    CommonState               CommonStateAllItems{};
    CommonState               CommonStateOwnItems{};
    CommonState               CommonStateSubItems{};
    TList_c<StepsItem>        Items{};
    TList_c<StepsCategory>    SubCategories{};
};
};
