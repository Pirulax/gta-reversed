#include "StdInc.h"

#include "HooksSortListStep.hpp"
#include <concepts>

namespace RHDebugModule {
HooksSortListStep::HooksSortListStep() {
    m_BufSortedCategories.reserve(128);
    m_BufSortedItems.reserve(512);
}

void HooksSortListStep::Process(StepsCategory& cat) noexcept {
    ZoneScoped;

    const auto SortList = []<typename T, std::predicate<T*, T*> Pred>(Pred && pred, TList_c<T>&list, std::vector<T*> buf) {
        buf.reserve(list.GetSize());
        while (!list.IsEmpty()) {
            buf.push_back(list.RemoveItem(list.GetHead()));
        }
        for (auto& v : list) {
            buf.push_back(&v);
        }
        rng::sort(buf, std::forward<Pred>(pred));
        for (auto* const v : buf) {
            list.AppendItem(v);
        }
        buf.clear();
    };

    if (cat.HasItems) {
        SortList([](const StepsItem* a, const StepsItem* b) {
            if (a->FilterScore.has_value() && b->FilterScore.has_value()) {
                return a->FilterScore < b->FilterScore;
            }
            return a->Ptr->GetName() < b->Ptr->GetName();
        }, cat.Items, m_BufSortedItems);
        //cat.Items.Sort([](const StepsItem& a, const StepsItem& b) {
        //    if (a.FilterScore.has_value() || b.FilterScore.has_value()) {
        //        return a.FilterScore < b.FilterScore;
        //    }
        //    return a.Ptr->GetName() < b.Ptr->GetName();
        //});
    }

    if (cat.HasSubCategories) {
        //SortList([](const StepsCategory* a, const StepsCategory* b) {
        //    if (a->MaxFilterScore.has_value() && b->MaxFilterScore.has_value()) {
        //        return a->MaxFilterScore > b->MaxFilterScore;
        //    }
        //    return a->Category->Name() < b->Category->Name();
        //}, cat.Categories, m_BufSortedCategories);
        cat.Categories.Sort([](const StepsCategory& a, const StepsCategory& b) {
            if (a.MaxFilterScore.has_value() && b.MaxFilterScore.has_value()) {
                return a.MaxFilterScore > b.MaxFilterScore;
            }
            return a.Category->Name() < b.Category->Name();
        });

        for (auto& v : cat.Categories) {
            Process(v);
        }
    }
}
}; // namespace RHDebugModule
