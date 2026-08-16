#include "StdInc.h"
#include "Base.h"
#include <iterator>
#include "HookFilter.h"

namespace RHDebugModule {
HookFilter::HookFilter(std::string_view untrimmedInput, bool caseSensitive, Cutoffs cutoffs) :
    m_IsCaseSensitive{ caseSensitive },
    m_Cutoffs{ cutoffs }
{
    const auto input = notsa::trim_string(untrimmedInput);

    if (input.empty()) {
        return;
    }

    const auto TrySetAddressFilter = [this](std::string_view addressStr) {
        if (addressStr.starts_with("0x") || addressStr.starts_with("0X")) {
            addressStr = addressStr.substr(2);
        }
        if (const auto address = notsa::try_ston<uintptr>(addressStr, 16)) {
            m_HookAddressFilter = GetPtrAsHexString(*address);
            return true;
        }
        return false;
    };

    // If the first character is a digit, we assume the user wants to filter by address
    // as namespace or function names can't start with a digit
    if (DIGITS.contains(input.front())) {
        TrySetAddressFilter(input);
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
        if (!hookFilterStr.empty() && hookFilterStr.find_first_of(INVALID_HOOK_FILTER_CHARS) == std::string_view::npos) {
            if (!TrySetAddressFilter(hookFilterStr)) {
                m_HookNameFilter = hookFilterStr;
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

float HookFilter::MatchItem(
    std::string_view name,
    void*            addressA,
    void*            addressB
) const noexcept {
    float max = 0.0;
    const auto DoTest = [&](std::string_view haystack, std::string_view needle, float cutoff) {
        max = std::max(max, MatchString(haystack, needle, cutoff, m_IsCaseSensitive));
    };
    if (IsHookFilterByNameActive()) {
        DoTest(name, *m_HookNameFilter, IsSimpleFilterString() ? m_Cutoffs.ItemGlobal : m_Cutoffs.ItemLocal);
    }
    if (IsHookFilterByAddressActive()) {
        for (auto address : { addressA, addressB }) {
            if (!address) {
                continue;
            }
            //char buf[64]{ "0x" };
            //const auto [end, ec] = std::to_chars(buf + 2, buf + sizeof(buf) - 2, (uintptr_t)(address), 16);
            //if (ec != std::errc{}) {
            //    continue;
            //}
            DoTest(GetPtrAsHexString(std::bit_cast<uintptr_t>(address)), *m_HookAddressFilter, m_Cutoffs.ItemAddress);
        }
    }
    return max;
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

float HookFilter::MatchCategoryByPath(const CategoryPath& path, size_t depth) const noexcept {
    assert(path.size() > depth);

    if (m_CategoryPathTokens.size() > depth + 1) {
        return 0.f; // Can't match whole, so just stop now
    }

    const auto MatchPath = [&] (size_t off = 0) {
        float total = 0.f;
        for (size_t i = 0; i < m_CategoryPathTokens.size(); i++) {
            const auto match = MatchString(path[off + i], m_CategoryPathTokens[i], m_Cutoffs.CategoryInPath, m_IsCaseSensitive, true);
            if (match <= 0.f) {
                return 0.f;
            }
            total += match;
        }
        return total;
    };

    if (IsRootRelativePath()) { // Root must match stricly match the beginning
        return MatchPath(0); // Compare whole from the begining of `path`
    } else { // This should match at the end
        if (m_CategoryPathTokens.size() > depth + 1) {
            return 0.f; // Can't match whole, so just stop now
        }
        return MatchPath(1 + depth - m_CategoryPathTokens.size()); // Compare `m_CategoryPathTokens` with the end of `path`
    }
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

std::string HookFilter::GetPtrAsHexString(uintptr_t ptr) const noexcept {
    // `to_chars` doesn't use the `0x` prefix, so we don't need to account for it here
    // by limiting the size of the string we avoid heap allocation
    // because std::string has SSO of around 10-15 chars on x32
    constexpr auto MAX_PTR_HEX_SIZE = sizeof(uintptr_t) * 2;

    std::string hex;
    hex.resize(MAX_PTR_HEX_SIZE);
    const auto [end, ec] = std::to_chars(hex.data(), hex.data() + hex.size(), ptr, 16);
    if (ec != std::errc{}) {
        return {};
    }
    hex.resize(rng::distance(hex.data(), end));
    return hex;
}
}; // namespace RHDebugModule
