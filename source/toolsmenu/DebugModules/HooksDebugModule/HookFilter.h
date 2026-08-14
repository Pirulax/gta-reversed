#pragma once

#include <optional>
#include <thread>
#include <string>
#include <string_view>
#include <boost/container/static_vector.hpp>
#include <boost/static_string.hpp>
#include <rapidfuzz/fuzz.hpp>
#include <rapidfuzz/distance/Levenshtein.hpp>

#include <toolsmenu/DebugModules/DebugModule.h>

class HookFilter {
    using StaticString = boost::static_string<512>;

    static constexpr std::string_view NAMESPACE_SEP{ "/" };
    static constexpr std::string_view INVALID_CAT_PATH_CHARS{ "!\"#$%&'()*+,-.:;<=>?@[\\]^`{|}~ " }; // Characters that may not be present in the namespace filter

    static constexpr std::string_view HOOK_FILTER_SEP{ "::" };
    static constexpr std::string_view DIGITS{ "0123456789" };
    static constexpr std::string_view INVALID_HOOK_FILTER_CHARS{ "!\"#$%&'()*+,-./:;<=>?@[\\]^`{|}~ " }; // Characters that may not be present in the hook filter

    using CachedRapidfuzz = rapidfuzz::CachedLevenshtein<char>;

public:
    using NamespaceTokens = boost::container::static_vector<std::string_view, 32>;

    struct Cutoffs {
        float Category{ 0.5f };
        float CategoryGlobal{ 0.5f };
        float ItemGlobal{ 0.5f };
        float ItemLocal{ 0.1f };
        float ItemAddress{ 0.7f };

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Cutoffs, Category, CategoryGlobal, ItemGlobal, ItemLocal, ItemAddress)
    };

public:
    HookFilter() = default;
    HookFilter(std::string_view input, bool caseSensitive, Cutoffs cutoffs = {});
    ~HookFilter();

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
    bool IsFilteringByCategoryPath() const noexcept;

    bool IsFilteringByCategory() const noexcept { return !m_CategoryPathTokens.empty(); }

    /*!
     * @param cat Category to check
     * @param path Path of the category, including the root `/`, eg.: `/Audio/AEVehicleAudioEntity` => `{ "", "Audio", "AEVehicleAudioEntity" }`
     * @param depth Depth of the category in the namespace hierarchy
     * @return The match score of the category, or `nullopt` if it doesn't match the filter
     */
    float MatchCategoryByNamespace(const NamespaceTokens& path, size_t depth) const noexcept;

    /**
     * @brief Checks if the category name matches the filter
     */
    float MatchCategoryByName(std::string_view name) const noexcept;

    /*!
     * @return If there are active filters, if false, nothing is filtered out
     */
    bool HasActiveFilters() const noexcept { return IsFilteringByCategoryPath() || IsHookFilterActive(); }

    // Check if hook filter is present.
    // even in case it's present it might be empty
    // in which case it wouldn't filter out anything.
    // Usually you want to use `IsHookFilterActive` which checks both.
    bool IsHookFilterPresent() const noexcept { return m_HookFilter.has_value(); }

    // Should the current filtered namespace be relative to the root namespace.
    // This is the case when the user prepends the namespace tokens with a `/` (NAMESPACE_SEP).
    // Eg.: `/Entity` should only show the `Entity` namespace under `Root` (But not, for example, `Audio/AEVehicleAudioEntity`)
    bool IsRootRelativeNamespace() const noexcept { return m_CategoryPathTokens.size() >= 1 && m_CategoryPathTokens.front().empty(); }

    bool IsFilteringByCategoryName() const noexcept { return m_CategoryPathTokens.size() == 1; }

    /*!
     * @return
     */
    bool IsSimpleFilterString() const noexcept { return m_IsSimpleFilterString; }

    /*!
     * @brief Do string matching using Levenshtein distance against the 
     */


private:
    float MatchString(
        std::string_view haystack,
        std::string_view needle,
        float            cutoff = 1.f,
        bool             caseSensitive = false
    ) const noexcept;

private:
    //! Contains all the tokens on the left side split by `NAMESPACE_SEP` of the input split by `HOOKNAME_SEP`
    //! Eg `m_input` => content:
    //! - `Name/Space/` => `Name`, `Space`, `` (<= empty string)
    //! - `Name/Space/::` => -||- (Same as the above example)
    //! - `/` - `` (empty string) - Indicates root namespace (See `IsRelativeToRootNamespace`)
    //! - `///` - 4 empty strings
    std::vector<std::string> m_CategoryPathTokens{};

    bool m_IsSimpleFilterString{};

    //! Filter of hook name or/and address (in hex form)
    //! If `nullopt` means there was no `::` (HOOK_FILTER_SEP) in the user input
    //! otherwise if there was, it contains whatever was after it (Which might be nothing - So the string is empty)
    std::optional<std::string> m_HookFilter{};

    //! If `m_HookFilter` is address-like (can be converted to a number from hex, with or without 0x prefix) and should be used to filter by address (as well)
    bool m_HookFilterByAddress{};

    //! If `m_HookFilter` should be used to filter by name (as well)
    bool m_HookFilterByName{};

    bool m_IsCaseSensitive{};
    Cutoffs m_Cutoffs{};
};
