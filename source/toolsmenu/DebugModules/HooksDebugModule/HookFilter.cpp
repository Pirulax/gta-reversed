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

float HookFilter::MatchCategoryByNamespace(const NamespaceTokens& path, size_t depth) const noexcept {
    assert(path.size() > depth);

    float total = 0.f;
    const auto Match = [&] (size_t idxHaystack, size_t idxNeedle) {
        const auto match = MatchString(path[idxHaystack], m_CategoryPathTokens[idxNeedle], m_Cutoffs.CategoryInPath, m_IsCaseSensitive, true);
        total += match;
        return match > 0.f;
    };

    if (IsRootRelativeNamespace()) { // Root must match stricly match the beginning, it's ok if there are more tokens
        if (depth == 0) {
            return 1.f; // Root will always match the beginning of the path at this depth
        }
        for (size_t i = 1; i < m_CategoryPathTokens.size(); i++) { // Start at 1, because 0 is root
            if (i > depth) { 
                break; // We can't match now, but might match later
            }
            if (!Match(i, i)) {
                return 0.f; // Part didn't match
            }
        }
    } else { // This should match at the end
        if (m_CategoryPathTokens.size() > depth + 1) {
            return 0.f; // Can't match whole, so just stop now
        }
        const auto off = (depth + 1) - m_CategoryPathTokens.size();
        for (size_t i = 0; i < m_CategoryPathTokens.size(); i++) {
            if (!Match(off + i, i)) {
                return 0.f; // Part didn't match
            }
        }
    }

    return total;
}

float HookFilter::MatchCategoryByName(std::string_view name) const noexcept {
    assert(m_CategoryPathTokens.size() == 1);
    return MatchString(name, m_CategoryPathTokens.front(), m_Cutoffs.CategoryGlobal, m_IsCaseSensitive);
}

float HookFilter::MatchString(std::string_view haystack, std::string_view needle, float cutoff, bool caseSensitive, bool emptyNeedleMatchesAll) const noexcept {
    if (needle.empty()) {
        return emptyNeedleMatchesAll ? 1.f : 0.f; // This way if user didn't type anything stuff will still show up
    } else if (needle == WILDCARD_CHAR) {
        return 1.f;
    }
    if (haystack.empty()) {
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
