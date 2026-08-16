#include "StdInc.h"

#include "HookFilter.h"
#include "RListBuilder.h"

namespace RHDebugModule {
RListBuilder::RListBuilder(
    size_t maxItems,
    size_t maxCategories
) :
    m_PoolItem{ maxItems },
    m_PoolCategory{ maxCategories } {
}

RListCategory* RListBuilder::ConstructList(std::shared_ptr<ReversibleHooks::HookCategory> from) noexcept {
    ZoneScoped;

    const auto RecursiveConstruct = [this](this auto& RecursiveConstruct, std::shared_ptr<ReversibleHooks::HookCategory> cat) -> RListCategory& {
        RListCategory* out = new (m_PoolCategory.New()) RListCategory{
            .Category{ std::move(cat) },
        };

        for (auto item : out->Category->Items()) {
            out->Items.AppendItem(
                new (m_PoolItem.New()) RListCategoryItem{
                    .Ptr{ std::move(item) },
                }
            );
        }

        for (auto sc : out->Category->SubCategories()) {
            if (sc->SubCategories().empty() && sc->Items().empty()) {
                continue; // Couldn't calculate common state, and also no sense to show empty categories in the UI
            }
            auto* const lsc = out->Categories.AppendItem(
                &RecursiveConstruct(std::move(sc))
            );
            UpdateCategory(*lsc);
        }
        out->NumCategoriesIgnored = out->Category->SubCategories().size() - out->Categories.GetSize();

        UpdateCategory(*out);

        return *out;
    };

    return &RecursiveConstruct(std::move(from));
}

bool RListBuilder::UpdateCategory(RListCategory& cat) const noexcept {
    ZoneScoped;

    const auto prevAnyUnhookedItems    = std::exchange(cat.AnyUnhookedItems, false);
    cat.AnyUnhookedOwnItems = false; // This is OR'd with the above, so both will change if either does
    const auto prevAnyUnlockedItems    = std::exchange(cat.AnyUnlockedItems, false);
    cat.AnyUnhookedOwnItems = false; // This is OR'd with the above, so both will change if either does
    const auto prevCommonStateAllItems = std::exchange(cat.CommonStateAllItems, std::nullopt);
    const auto prevCommonStateOwnItems = std::exchange(cat.CommonStateOwnItems, std::nullopt);
    const auto prevCommonStateSubItems = std::exchange(cat.CommonStateSubItems, std::nullopt);

    const auto GetCommonState          = [](CommonState base, CommonState other) -> CommonState {
        return base == other
            ? base
            : std::nullopt;
    };

    assert((cat.Items.GetSize() == cat.Category->Items().size()) && "New items were added, the code doesn't account for them!");
    if (!cat.Items.IsEmpty()) {
        cat.CommonStateOwnItems = cat.Items.GetHead()->Ptr->GetState();
        for (auto& item : cat.Items) {
            cat.CommonStateOwnItems = GetCommonState(cat.CommonStateOwnItems, item.Ptr->GetState());
            cat.AnyUnhookedOwnItems |= item.Ptr->GetState() == HookState::Unhooked;
            cat.AnyUnlockedOwnItems |= !item.Ptr->GetIsStateLocked();
        }
        cat.AnyUnhookedItems |= cat.AnyUnhookedOwnItems;
        cat.AnyUnlockedItems |= cat.AnyUnlockedOwnItems;
    }

    assert((cat.Categories.GetSize() + cat.NumCategoriesIgnored == cat.Category->SubCategories().size()) && "New sub-categories were added, code doesn't account for them!");
    for (auto&& [i, sc] : cat.Categories | rngv::enumerate) {
        cat.AnyUnhookedItems |= sc.AnyUnhookedItems;
        cat.AnyUnlockedItems |= sc.AnyUnlockedItems;
        cat.CommonStateSubItems = i == 0
            ? sc.CommonStateAllItems
            : GetCommonState(cat.CommonStateSubItems, sc.CommonStateAllItems);
    }

    cat.CommonStateAllItems =
        !cat.Items.IsEmpty() && !cat.Categories.IsEmpty() ? GetCommonState(cat.CommonStateOwnItems, cat.CommonStateSubItems)
        : !cat.Items.IsEmpty()                            ? cat.CommonStateOwnItems
        : !cat.Categories.IsEmpty()                       ? cat.CommonStateSubItems
                                                          : std::nullopt;

    return prevAnyUnhookedItems != cat.AnyUnhookedItems
        || prevAnyUnlockedItems != cat.AnyUnlockedItems
        || prevCommonStateAllItems != cat.CommonStateAllItems
        || prevCommonStateOwnItems != cat.CommonStateOwnItems
        || prevCommonStateSubItems != cat.CommonStateSubItems;
}
}; // namespace RHDebugModule
