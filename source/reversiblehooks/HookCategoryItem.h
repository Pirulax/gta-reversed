#pragma once

#include <fstream>
#include <memory>
#include <source_location>

#include <reversiblehooks/ReversibleHook/TwoWayHook.h>

namespace ReversibleHooks {
class HookCategory;

class HookCategoryItem {
    using HookPtr   = std::shared_ptr<ReversibleHook::TwoWayHook>;

public:
    using HookState = ReversibleHook::TwoWayHookState;

public:
    HookCategoryItem(
        HookPtr              hook,
        bool                 isLocked,
        bool                 isReversed,
        HookState            state,
        std::source_location installSrcLoc
    ) :
        m_Hook{ std::move(hook) },
        m_IsStateLocked{ isLocked },
        m_IsReversed{ isReversed },
        m_InstallSrcLoc{ std::move(installSrcLoc) }
    {
        SetState(state, true);
    }

    /*!
     * @return The 
     */
    auto GetHook() const noexcept { return m_Hook; }

    /*!
     * @return Hook's current state
     */
    auto GetState() const { return m_Hook->State(); }

    /*!
     * @brief Set state of hook
     * @param state new state
     * @param ignoreLock if true, will ignore the lock and set the state regardless of it
     * @return If the state has changed
     */
    bool SetState(HookState state, bool ignoreLock = false);

    /*!
     * @return Symbol to use for the hook in the UI
     */
    const char* GetTypeSymbolUI() noexcept;

    /*!
     * @brief Restore to previous state
     */
    bool SetToPreviousState() { return SetState(m_PrevState); }

    /*!
     * @returns The previous state of the hook (before the last change)
     */
    auto GetPreviousState() const noexcept { return m_PrevState; }

    /*!
     * @return If the hook should be shown in the UI 
     */
    bool GetMatchesSearchFilter() const noexcept { return m_MatchesSearchFilter; }

    /*!
     * @brief If the hook matches the current search criteria in the UI
     * @param matches New visibility
     */
    void SetMatchesSearchFilter(bool matches) noexcept { m_MatchesSearchFilter = matches; }

    /*!
     * @return If state is locked (and can't be changed)
     */
    bool GetIsStateLocked() const noexcept { return  m_IsStateLocked; }

    /*!
     * @return Hook's name
     */
    const auto& GetName() const noexcept { return m_Hook->Name(); }

    /*!
     * @return Hook's type
     */
    auto GetType() const noexcept { return m_Hook->Type(); }

    /*!
     * @return Hook's address in GTA's code (nullptr if not applicable)
     */
    void* GetHookAddressGTA() const noexcept { return m_Hook->GetHookAddressGTA(); }

    /*!
     * @return Hook's address in our code (nullptr if not applicable)
     */
    void* GetHookAddressOur() const noexcept { return m_Hook->GetHookAddressOur(); }

    /*!
     * @brief Prints the hook to a CSV file (Format: `class,fn_name,address,reversed,locked,type`)
     * @param cat Category the hook is in
     * @param ofs Output file stream to write the CSV data
     */
    void PrintToCSV(std::ofstream& ofs, const HookCategory& cat) const noexcept;

protected:
    HookState            m_PrevState{};
    std::source_location m_InstallSrcLoc;              //!< Source location where the hook was installed
    HookPtr              m_Hook;                       //!< The hook
    bool                 m_IsReversed : 1 {};          //!< Is re'd?
    bool                 m_IsStateLocked : 1 {};       //!< Is state locked (can't be changed by user)
    bool                 m_MatchesSearchFilter : 1 {}; //!< Mathces current UI search criteria and thus should be rendered
};
}; // namespace ReversibleHooks
