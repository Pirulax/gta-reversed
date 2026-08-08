#pragma once

#include <fstream>
#include <memory>
#include <source_location>

#include <reversiblehooks/ReversibleHook/TwoWayHookBase.h>

namespace ReversibleHooks {
class HookCategory;

class HookCategoryItem {
    using HookPtr = std::shared_ptr<ReversibleHook::TwoWayHookBase>;

public:
    HookCategoryItem(
        HookPtr              hook,
        bool                 isLocked,
        bool                 isReversed,
        bool                 isEnabled,
        std::source_location installSrcLoc
    ) :
        m_Hook{ std::move(hook) },
        m_IsStateLocked{ isLocked },
        m_IsReversed{ isReversed },
        m_InstallSrcLoc{ std::move(installSrcLoc) }
    {
        assert((m_IsReversed || !isEnabled) && "Hooks for functions not `reversed` should not be `enabled`");
        SetState(isEnabled, true);
    }

    /*!
     * @return The 
     */
    auto GetHook() const noexcept { return m_Hook; }

    /*!
     * @return Hook's current state
     */
    bool GetState() const { return m_Hook->Hooked(); }

    /*!
     * @brief Set state of hook
     * @param state new state
     * @param ignoreLock if true, will ignore the lock and set the state regardless of it
     * @return If the state has changed
     */
    bool SetState(bool state, bool ignoreLock = false) { return (ignoreLock || !m_IsStateLocked) && m_Hook->State(state); }

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
    void* GetHookAddressGTA() const noexcept;

    /*!
     * @return Hook's address in our code (nullptr if not applicable)
     */
    void* GetHookAddressOur() const noexcept;

    /*!
     * @return Hook's symbol (nullptr if not applicable)
     */
    const char* GetSymbol() const noexcept;

    /*!
     * @brief Prints the hook to a CSV file (Format: `class,fn_name,address,reversed,locked,type`)
     * @param cat Category the hook is in
     * @param ofs Output file stream to write the CSV data
     */
    void PrintToCSV(std::ofstream& ofs, const HookCategory& cat) const noexcept;

protected:
    std::source_location m_InstallSrcLoc;              //!< Source location where the hook was installed
    HookPtr              m_Hook;                       //!< The hook
    bool                 m_IsReversed : 1 {};          //!< Is re'd?
    bool                 m_IsStateLocked : 1 {};       //!< Is state locked (can't be changed by user)
    bool                 m_MatchesSearchFilter : 1 {}; //!< Mathces current UI search criteria and thus should be rendered
};
}; // namespace ReversibleHooks
