#pragma once

#include <optional>
#include <thread>
#include <string>
#include <string_view>
#include <boost/container/static_vector.hpp>

#include <toolsmenu/DebugModules/DebugModule.h>

class HookFilter {
    static constexpr std::string_view NAMESPACE_SEP{ "/" };
    static constexpr std::string_view INVALID_NAMESPACE_FILTER_CHARS{ "!\"#$%&'()*+,-.:;<=>?@[\\]^`{|}~ " }; // Characters that may not be present in the namespace filter

    static constexpr std::string_view HOOK_FILTER_SEP{ "::" };
    static constexpr std::string_view DIGITS{ "0123456789" };
    static constexpr std::string_view INVALID_HOOK_FILTER_CHARS{ "!\"#$%&'()*+,-./:;<=>?@[\\]^`{|}~ " }; // Characters that may not be present in the hook filter

public:
    using NamespaceTokens = boost::container::static_vector<std::string_view, 32>;

public:
    HookFilter(std::string_view input);

    /*!
     * @return If filter applies to items
     */
    bool IsHookFilterActive() const noexcept;

    /*!
     * @return The match score of the item, or `nullopt` if it doesn't match the filter
     */
    float MatchItem(
        std::string_view name,
        void*            addressA,
        void*            addressB
    ) const noexcept;

    /*!
     * @return If filter applies to categories
     */
    bool IsNamespaceFilterActive() const noexcept;

    /*!
     * @param cat Category to check
     * @param path Path of the category, including the root `/`, eg.: `/Audio/AEVehicleAudioEntity` => `{ "", "Audio", "AEVehicleAudioEntity" }`
     * @param depth Depth of the category in the namespace hierarchy
     * @return The match score of the category, or `nullopt` if it doesn't match the filter
     */
    float MatchCategoryByNamespace(
        const NamespaceTokens& path,
        size_t               depth
    ) const noexcept;

    /**
     * @brief Checks if the category name matches the filter
     */
    float MatchCategoryByName(
        std::string_view name
    ) const noexcept;

    /*!
     * @return If there are active filters, if false, nothing is filtered out
     */
    bool HasActiveFilters() const noexcept { return IsNamespaceFilterActive() || IsHookFilterActive(); }

    // Check if hook filter is present.
    // even in case it's present it might be empty
    // in which case it wouldn't filter out anything.
    // Usually you want to use `IsHookFilterActive` which checks both.
    bool IsHookFilterPresent() const noexcept { return m_HookFilter.has_value(); }

    // Should the current filtered namespace be relative to the root namespace.
    // This is the case when the user prepends the namespace tokens with a `/` (NAMESPACE_SEP).
    // Eg.: `/Entity` should only show the `Entity` namespace under `Root` (But not, for example, `Audio/AEVehicleAudioEntity`)
    bool IsRootRelativeNamespace() const noexcept { return m_NamespaceTokens.size() >= 1 && m_NamespaceTokens.front().empty(); }

    /*!
     * @return
     */
    bool IsSimpleGlobalSearch() const noexcept { return m_IsSimpleGlobalFilter; }

private:
    //! Are the string checks case sensitive?
    //! It is used, but there's no GUI to change it for now. It is case-insensitive by default (false).
    bool m_IsCaseSensitive{};

    //! Contains all the tokens on the left side split by `NAMESPACE_SEP` of the input split by `HOOKNAME_SEP`
    //! Eg `m_input` => content:
    //! - `Name/Space/` => `Name`, `Space`, `` (<= empty string)
    //! - `Name/Space/::` => -||- (Same as the above example)
    //! - `/` - `` (empty string) - Indicates root namespace (See `IsRelativeToRootNamespace`)
    //! - `///` - 4 empty strings
    std::vector<std::string_view> m_NamespaceTokens{};

    bool m_IsSimpleGlobalFilter{};

    //! Filter of hook name or/and address (in hex form)
    //! If `nullopt` means there was no `::` (HOOK_FILTER_SEP) in the user input
    //! otherwise if there was, it contains whatever was after it (Which might be nothing - So the string is empty)
    std::optional<std::string_view> m_HookFilter{};

    //! If `m_HookFilter` is address-like (can be converted to a number from hex, with or without 0x prefix) and should be used to filter by address (as well)
    bool m_HookFilterByAddress{};

    //! If `m_HookFilter` should be used to filter by name (as well)
    bool m_HookFilterByName{};
};
