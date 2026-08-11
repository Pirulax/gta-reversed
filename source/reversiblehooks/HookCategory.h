#pragma once

#include <ranges>
#include <array>
#include <list>
#include <string>
#include <algorithm>

#include <extensions/utility.hpp>
#include <TristateCheckbox.h>

#include "ReversibleHooks.h"
#include "HookCategoryItem.h"

namespace rng = std::ranges;

namespace ReversibleHooks {


class HookCategory {
public:
    using UIState = ImGui::ImTristate; // Don't really want to deal with enum conversions so this should do.
    using HookState = ReversibleHook::TwoWayHookState;

    enum class eOverallState {
        None,
        Mixed,

    };

    using CommonItemsState = std::optional<HookState>;
    
    // bool IsSame
    // State CommonState

    //enum class HooksState {
    //    ALL  = 1, // All GetState
    //    NONE = 0, // None GetState
    //    SOME = -1 // Some GetState
    //};

public:
    HookCategory(std::string name, HookCategory* parent) :
        m_name{ std::move(name) },
        m_parent{parent}
    {
    }

    // Accessors
    auto OverallState()         const { return m_CommonAllItemsState; }
    bool HasAnyUnhooked() const noexcept { return m_AnyOurItemsUnhooked || m_AnySubItemsUnhooked; }

    auto ItemsState()           const { return m_CommonOurItemsState; }
                                
    auto SubcategoriesState()   const { return m_CommonSubItemsState; }
                                
    const auto& Name()          const { return m_name; }
                                
    const auto& SubCategories() const { return m_subCategories; }
    auto&       SubCategories()       { return m_subCategories; }
                                
    const auto& Items()         const { return m_items; }
    auto&       Items()               { return m_items; }
                                
    auto Parent()               const { return m_parent; }

    bool Visible()              const { return m_isVisible; }
    void Visible(bool visible)        { m_isVisible = visible; }

    bool Open()                 const { return m_isOpen; }
    void Open(bool open)              { m_isOpen = open; }

    bool IsEmpty() const noexcept { return m_items.empty() && m_subCategories.empty(); }

    /*!
     * @return If all items (including those of sub-categories) are locked
     */
    bool IsLocked() const noexcept {
        if (!AreItemsLocked()) {
            return false;
        }
        return rng::all_of(m_subCategories, [](const auto& cat) {
            return cat.IsLocked();
        });
    }

    /*!
     * @return If all of our items are locked
     */
    bool AreItemsLocked() const {
        return m_AllItemsLocked;
    }

    //! Add one item to this category (Deal with possible state change)
    void AddItem(HookCategoryItem item) {
        assert(!FindItem(item.GetName()) && "Category already contains an item with such name");

        m_AllItemsLocked = (m_AllItemsLocked || m_items.empty()) && item.GetIsStateLocked();

        // Lexographically sorted insert 
        const auto& emplacedItem = *m_items.emplace(
            rng::upper_bound(m_items, item.GetName(), {}, [](auto&& v) { return v.GetName(); }),
            std::move(item)
        );


        // Re-calculate items state with this item now added
        OnItemStateChanged(emplacedItem);
    }

    template<rng::forward_range R>
    static CommonItemsState GetCommonState(
        R&&              items,
        auto&&           GetState
    ) {
        if (!items.empty()) {
            const auto state = std::invoke(GetState, items.front());
            if (rng::all_of(items | rng::views::drop(1), [&](const auto& item) {
                return std::invoke(GetState, item) == state;
            })) {
                return state; // Whole range has the same state
            }
        }
        return std::nullopt; // No items or mixed state
    }

    template<rng::forward_range R>
    bool SetStates(
        R&&               range,
        CommonItemsState& inOutCommonState,
        auto&&            SetState,
        auto&&            GetState,
        bool              recalculateOverallState = true,
        bool              notifyParent            = true
    ) {
        // Apply new state, see if any changed
        if (!rng::fold_left(range, false, [&](bool changed, auto& item) {
            return std::invoke(SetState, item) || changed;
        })) {
            return false; // No state has changed
        }

        // State has changed, calculate new
        const auto changed = std::exchange(inOutCommonState, GetCommonState(range, GetState)) != inOutCommonState;
        if (changed) {
            if (recalculateOverallState) {
                ReCalculateOverallStateAndMaybeNotify();
            }
        }
        return changed;
    }

public:
    void SetAllItemsToPreviousState() {
        ISetAllItemsState([](HookCategoryItem& item) {
            return item.SetToPreviousState();
        });
    }

    void SetOurItemsToPreviousState() {
        ISetOurItemsState([](HookCategoryItem& item) {
            return item.SetToPreviousState();
        });
    }

public:
    //! Set state of _our_ items at once (subcategories' items _excluded_)
    void SetOurItemsState(HookState state) {
        m_LastSetOurState = state;
        ISetOurItemsState([state](HookCategoryItem& i) { return i.SetState(state); });
    }

private:
    //! Set state of _our_ items at once (subcategories' items _excluded_)
    bool ISetOurItemsState(auto&& SetState, bool recalculateOverallState = true, bool notifyParent = false) {
        return SetStates(
            m_items,
            m_CommonOurItemsState,
            SetState,
            &HookCategoryItem::GetState,
            recalculateOverallState,
            notifyParent
        );
    }

private:
    //! Set sub-items state at once (our items _excluded_)
    bool ISetSubItemsState(auto&& SetState, bool recalculateOverallState = true, bool notifyParent = false) {
        return SetStates(
            m_subCategories,
            m_CommonOurItemsState,
            [&](HookCategory& cat) { return cat.ISetAllItemsState(SetState, recalculateOverallState, notifyParent); },
            &HookCategory::OverallState,
            recalculateOverallState,
            notifyParent
        );
    }

public:

    //! Set sub-items state at once (our items _excluded_)
    void SetSubItemsState(HookState state) {
        ISetSubItemsState([state](HookCategoryItem& item) { return item.SetState(state); });
    }

private:
    //! Set all items state at once (subcategories' items included)
    bool ISetAllItemsState(auto&& SetState, bool recalculateOverallState = true, bool notifyParent = false) {
        bool changed = false;
        changed |= ISetOurItemsState(SetState, true, false);
        changed |= ISetSubItemsState(SetState, true, false);
        if (changed && recalculateOverallState) {
            ReCalculateOverallStateAndMaybeNotify(notifyParent);
        }
        return changed;
    }

public:
    //! Set all items state at once (subcategories' items included)
    void SetAllItemsState(HookState state) {
        m_LastSetAllState = state;
        ISetAllItemsState([state](HookCategoryItem& item) { return item.SetState(state); });
    }

    // Set one item's state - Calling `item.SetState` isn't advised as the category's state won't be updated.
    bool SetItemState(HookCategoryItem& item, HookState state) {
        if (!item.SetState(state)) { // State changed?
            return false;
        }
        OnItemStateChanged(item);
        return true;
    }

    //! Find item by name (function name)
    HookCategoryItem* FindItem(std::string_view name) {
        const auto it = rng::find_if(m_items, [&](const auto& c) { return c.GetName() == name; });
        return it == m_items.end() ? nullptr : &*it;
    }

    //! Find subcategory by name (Only checks for direct sub categories [No recursion])
    HookCategory* FindSubcategory(std::string_view name) {
        const auto it = rng::find_if(m_subCategories, [&](const auto& c) { return c.Name() == name; });
        return it == m_subCategories.end() ? nullptr : &*it;
    }

    //! Find subcategory by name - Only checks for top-level children
    //! If none found a sub-category will be created with the given name.
    auto& FindOrCreateSubcategory(std::string_view name) {
        if (auto* cat = FindSubcategory(name))
            return *cat; // Return found

        // Insert it - It will be sorted later
        return m_subCategories.emplace_back(std::string{ name }, this);
    }

    //! Iterates over all items, including those in all subcategories
    //! NOTE: Make sure the function doesn't add/remove items/subcategories!
    //!       (Underlaying storages are vectors, which don't like being modifies while they're being iterated over)
    template<typename Fn>
    void ForEachItem(Fn&& fn) {
        for (auto& item : m_items) {
            std::invoke(fn, item);
        }
        for (auto& cat : m_subCategories) {
            cat.ForEachItem(fn);
        }
    }

    /*!
    * @brief Iterate over all sub-categories recursively
    *
    * @param fn A functor taking a `HookCategory&` as it's first argument
    */
    template<typename Fn>
    void ForEachCategory(Fn&& fn) {
        std::invoke(fn, *this);
        for (auto& cat : m_subCategories) {
            cat.ForEachCategory(fn);
        }
    }

    //! Called when `InjectHooksMain` has finished - That is, all hooks have been injected.
    //! From this point on no items/categories should be added/removed.
    void OnInjectionEnd() {
        // Re-sort all categories (uses operator<=> defined below)
        m_subCategories.sort();

        // Propagate to all child
        for (auto&& cat : m_subCategories) {
            cat.OnInjectionEnd();
        }

        // Call this only after the above (Because by then all sub-categories have their disabled state calculated properly)
        CalculateIsDisabled();
    }

private:
    /*!
    * @brief Calculates and caches disabled state
    * @returns If all items (including those of sub-category's) is disabled
    */
    void CalculateIsDisabled() {
        // Update subcategories first
        rng::for_each(m_subCategories, &HookCategory::CalculateIsDisabled);

        // Now update ourselves
        m_allCatsDisabled = rng::all_of(m_subCategories, [](HookCategory& cat) {
            return cat.IsLocked();
        });
        m_AllItemsLocked  = rng::all_of(m_items, [](const HookCategoryItem& item) {
            return item.GetIsStateLocked();
        });
    }

    CommonItemsState CalcualteAllItemsState() const {
        if (m_items.empty() && m_subCategories.empty()) {
            return std::nullopt;
        }
        if (m_items.empty()) {
            return m_CommonSubItemsState;
        }
        if (m_subCategories.empty()) {
            return m_CommonOurItemsState;
        }
        return m_CommonSubItemsState == m_CommonOurItemsState
            ? m_CommonOurItemsState
            : std::nullopt;
    }

    /*!
    * @brief Recalculates overall state, and if changed, notifies parent.
    *
    * @param notifyParent If the parent should be notified of the state change
    * 
    * @return If the overall state has changed
    */
    bool ReCalculateOverallStateAndMaybeNotify(bool notifyParent = true) {
        auto changed = false;
        changed |= std::exchange(m_CommonAllItemsState, CalcualteAllItemsState()) != m_CommonAllItemsState;
        changed |= std::exchange(m_AnyItemsUnhooked, m_AnyOurItemsUnhooked || m_AnySubItemsUnhooked) != m_AnyItemsUnhooked;
        if (notifyParent && changed) {
            if (m_parent) { // Now, notify parent (so they can update their state)
                m_parent->OnSubcategoryStateChanged(*this);
            }
        }
        return changed;
    }

    // Called when a sub-category's overall state changes
    // (Will propagate to parent if it affected this category's overall state)
    void OnSubcategoryStateChanged(HookCategory& cat) {
        if (cat.OverallState() != m_CommonSubItemsState) {
            m_CommonSubItemsState = GetCommonState(
                m_subCategories,
                &HookCategory::OverallState
            );
        }
        m_AnySubItemsUnhooked = cat.OverallState() == HookState::Unhooked || m_CommonSubItemsState == HookState::Unhooked || rng::any_of(m_subCategories, [](const HookCategory& c) {
            return c.m_AnyItemsUnhooked;
        });
        ReCalculateOverallStateAndMaybeNotify();
    }

    // Not always called - Only when an individual item's state changes
    // Also not called if item is modified from the outside
    // (Currently only called by `AddItem` and `SetItemState`, but not from `SetAllItemsState`)
    void OnItemStateChanged(const HookCategoryItem& item) {
        if (item.GetState() == m_CommonOurItemsState) {
            return; // No change
        }
        m_CommonOurItemsState = GetCommonState(
            m_items,
            &HookCategoryItem::GetState
        );
        m_AnyOurItemsUnhooked = item.GetState() == HookState::Unhooked || m_CommonOurItemsState == HookState::Unhooked || rng::any_of(m_items, [](const auto& i) {
            return i.GetState() == HookState::Unhooked;
        });
        ReCalculateOverallStateAndMaybeNotify();
    }

    // Main ordering criteria is the no. of top-level sub categories
    // Secondary criteria is the name
    friend std::weak_ordering operator<=>(const HookCategory& lhs, const HookCategory& rhs) {
        const auto numSubCatOrder = rhs.SubCategories().size() <=> lhs.SubCategories().size();
        if (std::is_eq(numSubCatOrder)) 
            return rhs.Name() <=> lhs.Name(); // Same number of sub-categories, order by name
        return numSubCatOrder; // Order by no. of sub-categories
    }

public:
    std::optional<HookState> m_LastSetAllState{};     //!< Used by the UI when OverallState is mixed to decide what the next state should be
    std::optional<HookState> m_LastSetOurState{};     //!< Used by the UI when ItemsState is mixed to decide what the next state should be
    CommonItemsState         m_CommonOurItemsState{}; //!< Common state of all items, or nullopt if they differ (Mixed)
    CommonItemsState         m_CommonSubItemsState{}; //!< Common state of all sub-categorie's items, or nullopt if they differ (Mixed)
    CommonItemsState         m_CommonAllItemsState{}; //!< Common state of all items and sub-categories, or nullopt if they differ (Mixed)
    bool                     m_AnyOurItemsUnhooked{ false };
    bool                     m_AnySubItemsUnhooked{ false };
    bool                     m_AnyItemsUnhooked{ false }; // True if any of our items or sub-categories' items are unhooked
    bool                     m_isVisible{ true };       // Updated each time the search box is updated. Indicates whenever we should be visible in the GUI.
    bool                     m_isOpen{};                // Is our tree currently open
    bool                     m_triStateToggle{};        // Used by the UI when m_overallState is mixed to decide what the next state should be
    bool                     m_anyItemsVisible{ true }; // Used when searching
    bool                     m_allCatsDisabled{ true }; // Are all owned sub-categories disabled
    bool                     m_AllItemsLocked{ true };  // Are all owned items disabled

private:
    HookCategory*               m_parent{};        // Category we belong to - In case of `RootHookCategory` this is always `nullptr`.
    std::string                 m_name{};          // Name of this category (Eg.: `Root`, `Global`, `Entity`, etc...)
    std::list<HookCategory>     m_subCategories{}; // Subcategories - It has to be a list, because we have links between categories
    std::list<HookCategoryItem> m_items{};         // Hooks in this category (`RootCategory` should have none)
};

}; // namespace ReversibleHooks 
