#include "StdInc.h"

#include "HookFilter.h"
#include "HooksBuildListStep.h"

namespace RHDebugModule {
HooksBuildListStep::HooksBuildListStep(
    size_t maxItems,
    size_t maxCategories
) :
    m_PoolItem{ maxItems },
    m_PooStepCategory{ maxCategories }
{
}

StepsCategory* HooksBuildListStep::ConstructList(std::shared_ptr<ReversibleHooks::HookCategory> cat) noexcept {
    ZoneScoped;

    const auto GetCommonState = [](CommonState base, CommonState other) -> CommonState {
        return base == other
            ? base
            : std::nullopt;
    };

    StepsCategory* out = new (m_PooStepCategory.New()) StepsCategory{
        .Category{ std::move(cat) },
    };

    out->HasItems = !out->Category->Items().empty();
    if (out->HasItems) {
        ZoneScoped;
        out->CommonStateOwnItems = out->Category->Items().front()->GetState();
        for (auto item : out->Category->Items()) {
            out->CommonStateOwnItems = GetCommonState(out->CommonStateOwnItems, item->GetState());
            out->AnyUnhookedItems |= item->GetState() == HookState::Unhooked;
            out->AnyUnlockedItems |= !item->GetIsStateLocked();
            out->Items.AppendItem(
                new (m_PoolItem.New()) StepsItem{
                    .Ptr{ std::move(item) },
                }
            );
        }
    }

    out->HasSubCategories = !out->Category->SubCategories().empty();
    if (out->HasSubCategories) {
        ZoneScoped;
        for (auto v : out->Category->SubCategories()) {
            if (!v->SubCategories().empty() && !v->Items().empty()) {
                continue; // Couldn't calculate common state, and also no sense to show empty categories in the UI
            }
            auto* const sc = out->Categories.AppendItem(
                ConstructList(std::move(v))
            );
            out->AnyUnhookedItems |= sc->AnyUnhookedItems;
            out->AnyUnlockedItems |= sc->AnyUnlockedItems;
            out->CommonStateSubItems = GetCommonState(out->CommonStateSubItems, sc->CommonStateAllItems);
        }
    }

    out->CommonStateAllItems =
        out->HasItems && out->HasSubCategories ? GetCommonState(out->CommonStateOwnItems, out->CommonStateSubItems)
        : out->HasItems                        ? out->CommonStateOwnItems
        : out->HasSubCategories                ? out->CommonStateSubItems
                                               : std::nullopt;

    return out;
}

}; // namespace RHDebugModule
