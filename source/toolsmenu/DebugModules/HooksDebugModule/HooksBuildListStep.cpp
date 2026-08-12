#include "StdInc.h"

#include "HookFilter.h"
#include "HooksBuildListStep.h"

namespace RHDebugModule {
HooksBuildListStep::HooksBuildListStep(
    std::shared_ptr<ReversibleHooks::HookCategory> cat,
    size_t maxItems,
    size_t maxCategories
) :
    m_PoolItem{ maxItems },
    m_PooStepCategory{ maxCategories }
{
    m_Result = ConstructList(std::move(cat));
}

StepsCategory* HooksBuildListStep::ConstructList(std::shared_ptr<ReversibleHooks::HookCategory> cat) noexcept {
    ZoneScoped;

    StepsCategory* out = new (m_PooStepCategory.New()) StepsCategory{
        .Name{ cat->Name() }
    };

    const auto GetCommonState = [](CommonState base, CommonState other) -> CommonState {
        return base == other
            ? base
            : std::nullopt;
    };

    out->HasItems = !cat->Items().empty();
    if (out->HasItems) {
        out->CommonStateOwnItems = cat->Items().front()->GetState();
        for (auto item : cat->Items()) {
            out->CommonStateOwnItems = GetCommonState(out->CommonStateOwnItems, item->GetState());
            out->AnyUnhookedItems |= item->GetState() == HookState::Unhooked;
            out->AnyUnlockedItems |= !item->GetIsStateLocked();
            out->Items.AppendItem(
                new (m_PoolItem.New()) StepsItem{
                    .Name{ item->GetName() },
                    .Item{ std::move(item) },
                }
            );
        }
    }

    out->HasSubCategories = !cat->SubCategories().empty();
    if (out->HasSubCategories) {
        for (auto v : cat->SubCategories()) {
            if (!v->SubCategories().empty() && !v->Items().empty()) {
                continue; // Couldn't calculate common state, and also no sense to show empty categories in the UI
            }
            auto* const sc = out->SubCategories.AppendItem(
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
