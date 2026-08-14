#include "StdInc.h"
#include "Base.h"
#include <imgui.h>
#include <libs/imgui/misc/cpp/imgui_stdlib.h>
#include <boost/static_string.hpp>
#include <iterator>


#include "HookFilter.h"

HookFilter::HookFilter(std::string_view untrimmedInput, bool caseSensitive, Cutoffs cutoffs) :
    m_IsCaseSensitive{ caseSensitive },
    m_Cutoffs{ cutoffs }
{
    const auto input = notsa::trim_string(untrimmedInput);

    if (input.empty()) {
        return;
    }

    // If the first character is a digit, we assume the user wants to filter by address
    // as namespace or function names can't start with a digit
    if (DIGITS.contains(input.front())) {
        m_HookFilter          = input;
        m_HookFilterByAddress = true;
    } else {
        const auto namespaceSepPos = input.rfind(HOOK_FILTER_SEP);
        const auto hasNamespaceSep = namespaceSepPos != std::string_view::npos;

        // First half (if any), or the whole input is the category path filter
        const auto categoryPathStr = hasNamespaceSep
            ? input.substr(0, namespaceSepPos)
            : input;
        if (!categoryPathStr.contains(INVALID_CAT_PATH_CHARS)) {
            for (auto t : SplitStringView(categoryPathStr, NAMESPACE_SEP)) {
                m_CategoryPathTokens.emplace_back(notsa::trim_string(t));
            }
            if (m_CategoryPathTokens.size() == 1 && m_CategoryPathTokens.front().empty()) {
                m_CategoryPathTokens.clear(); // An empty string would match all strings, so there's no point in keeping it
            }
            if (m_CategoryPathTokens.size() > 1 && m_CategoryPathTokens.front().empty()) { // Eg.: `/...`
                m_CategoryPathTokens[0] = ReversibleHooks::RootHookCategory::GetRootName(); // This corresponds to the root namespace filter
            }
        }
        // Second half (if any) or the whole input is the hook filter
        const auto hookFilterStr = hasNamespaceSep
            ? notsa::trim_string(input.substr(namespaceSepPos + HOOK_FILTER_SEP.size()))
            : input;
        if (hookFilterStr.find_first_of(INVALID_HOOK_FILTER_CHARS) == std::string_view::npos) {
            m_HookFilter = hookFilterStr;
            if (!hookFilterStr.empty()) {
                m_HookFilterByName    = !DIGITS.contains(hookFilterStr.front()); // Names can't start with a digit, if they do so, it might be a hex address
                m_HookFilterByAddress = !m_HookFilterByName || notsa::try_ston<uintptr>(hookFilterStr, 16).has_value();
            }
        }

        m_IsSimpleFilterString = !hasNamespaceSep && IsFilteringByCategoryName() && IsHookFilterActive();
    }

    // In case user passes in a string with multiple `/` with nothing in-between we will have quite a few empty tokens.
    // We have have to remove all the leading empty tokens up until the last empty one.
    while (m_CategoryPathTokens.size() >= 2 && m_CategoryPathTokens[0].empty() && m_CategoryPathTokens[1].empty()) {
        m_CategoryPathTokens.erase(m_CategoryPathTokens.begin());
    }
}

HookFilter::~HookFilter() {
    NOTSA_LOG_DEBUG("des");
}


bool HookFilter::IsHookFilterActive() const noexcept { 
    return IsHookFilterPresent() && !m_HookFilter->empty(); 
}

float HookFilter::MatchItem(
    std::string_view lowerCaseName,
    void*            addressA,
    void*            addressB
) const noexcept {
    float score = 0.0;
    const auto DoTest = [&](std::string_view str, float cutoff) {
        score = std::max(score, MatchString(str, *m_HookFilter, cutoff, m_IsCaseSensitive));
    };
    if (m_HookFilterByName) {
        DoTest(lowerCaseName, IsSimpleFilterString() ? m_Cutoffs.ItemGlobal : m_Cutoffs.ItemLocal);
    }
    if (m_HookFilterByAddress) {
        for (auto address : { addressA, addressB }) {
            if (!address) {
                continue;
            }
            char buf[64]{ "0x" };
            const auto [end, ec] = std::to_chars(buf + 2, buf + sizeof(buf) - 2, (uintptr_t)(address), 16);
            if (ec != std::errc{}) {
                continue;
            }
            DoTest({ buf, end }, m_Cutoffs.ItemAddress);
        }
    }
    return static_cast<float>(score);
}

bool HookFilter::IsFilteringByCategoryPath() const noexcept {
    if (m_CategoryPathTokens.empty()) {
        return false; // No tokens
    }
    if (IsFilteringByCategoryName()) {
        return false; // Only filtering by category name, not path
    }
    return true;
}

float HookFilter::MatchCategoryByNamespace(const NamespaceTokens& ns, size_t depth) const noexcept {
    //const auto MatchRanges = [&] (int32 start, int32 stop, int32 step) {
    //    for (int32 i = start; i < stop; i += step) { // 10
    //           
    //    }
    //};

    float total = 0.f;
    const auto Match = [&, cutoff = IsSimpleFilterString() ? m_Cutoffs.CategoryGlobal : m_Cutoffs.Category] (size_t i) {
        const auto match = MatchString(ns[i], m_CategoryPathTokens[i], cutoff, m_IsCaseSensitive);
        total += match;
        return match > 0.f;
    };

    if (IsRootRelativeNamespace()) { // Root must match from the front -> back
        for (size_t i = 0; i < m_CategoryPathTokens.size(); i++) {
            if (i >= ns.size()) { 
                break; // We can't match now, but might match later
            }
            if (!Match(i)) {
                return 0.f; // Part didn't match
            }
        }
    } else { // This should match from back -> front
        for (size_t i = m_CategoryPathTokens.size(); i --> 0;) {
            if (i >= ns.size()) { 
                return false; // Can't match whole, so just stop now
            }
            if (!Match(i)) {
                return 0.f; // Part didn't match
            }
        }
    }

    return total;


    //const auto Match = [&](size_t i) {
    //    return MatchString(ns[i], m_NamespaceTokens[i], m_Cutoff, m_IsCaseSensitive);
    //    //return StringContainsString(name, m_NamespaceTokens[nsTokenIdx], m_IsCaseSensitive)
    //    //    ? 1.0f
    //    //    : 0.f;
    //};
    //if (IsRootRelativeNamespace()) {
    //    if (depth < m_NamespaceTokens.size()) {
    //        return 0.f; // Not enough tokens to match
    //    }
    //    return Match(depth);
    //}
    //return rng::fold_left(rng::views::iota(0u, m_NamespaceTokens.size()), 0.f, [&](float max, size_t i) {
    //    if (depth + i >= ns.size()) {
    //        return max; // Not enough tokens to match
    //    }
    //    return std::max(max, Match(depth + i));
    //});
    //float match = 0.f;
    //for (size_t i = 0; i < m_NamespaceTokens.size(); ++i) {
    //    if (depth + i >= ns.size()) {
    //        break; // Not enough tokens to match
    //    }
    //    match = std::max(match, Match(depth + i));
    //}
    //return match;

    //const auto Match = [this](const auto& range) -> float {
    //    float score = 0.f;
    //    for (auto&& [haystack, needle] : range) { // We don't check root, drop 1
    //        if (!StringContainsString(haystack, needle, m_IsCaseSensitive)) {
    //            return std::nullopt;
    //        }
    //        score += 1.f;
    //    }
    //    return score;
    //};

   //if (IsRootRelativeNamespace()) { // Check if path matches from the beginning
    //    if (path.size() < m_NamespaceTokens.size()) {
    //        return std::nullopt; // Not enough tokens to match
    //    }
    //    return Match(rngv::zip(path, m_NamespaceTokens) | rngv::drop(1)); // We don't check root, drop 1
    //} else { // Check if filter matches path from the end
    //    if (path.size() < m_NamespaceTokens.size()) {
    //        return std::nullopt; // Not enough tokens to match
    //    }

   // //    for 
    //}
}

float HookFilter::MatchCategoryByName(std::string_view name) const noexcept {
    assert(m_CategoryPathTokens.size() == 1);
    return MatchString(name, m_CategoryPathTokens.front(), m_Cutoffs.CategoryGlobal, m_IsCaseSensitive);
}

float HookFilter::MatchString(std::string_view haystack, std::string_view needle, float cutoff, bool caseSensitive) const noexcept {
    if (haystack.empty() || needle.empty()) {
        return 0.f;
    }
    const auto CalculateScore = [&] (size_t pos) {
        if (pos == std::string_view::npos) {
            return 0.f;
        }
        if (haystack.size() == needle.size()) {
            return 1.f;
        }
        const auto score = (float)(needle.size()) / (float)(haystack.size() + pos);
        return score >= cutoff ? score : 0.f;
        };
    if (caseSensitive) {
        return CalculateScore(haystack.find(needle));
    }
    const auto ToUpper = [](auto&& c) {
        return (char)(std::toupper((unsigned char)(c)));
        };
    const auto range = rng::search(haystack, needle, {}, ToUpper, ToUpper);
    if (range.empty()) {
        return 0.f;
    }
    return CalculateScore(rng::distance(haystack.begin(), range.begin()));
}
