#include "StdInc.h"

#include <imgui.h>
#include <libs/imgui/misc/cpp/imgui_stdlib.h>

#include "HookFilter.h"

//void HookFilter::Draw() {
//    ImGui::PushItemWidth(ImGui::GetWindowContentRegionMax().x - 10.f);
//    if (ImGui::InputTextWithHint(
//        "##HookFilter",
//        "`::function`         - Filters only functions \n"
//        "`cpy`                - Filter namespace - Will only show namespace with name containing \"cphy\"\n"
//        "`ped/player`         - Should only show Ped/CPlayerPed\n"
//        "`ped/player::busted` - Should only show `Ped/CPlayerPed` with the `busted` function visible only\n"
//        "`/entity`            - Should only show the top level `Entity` namespace in Root\n"
//        "For more tips see gta-reversed-modern/discussions/190\n",
//        &m_Input
//    )) {
//        CalculateFilterFromInput();
//    }
//    ImGui::PopItemWidth();
//}

HookFilter::HookFilter(std::string_view input) {
    input = notsa::trim_string(input);
    
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

        // First half (if any), or the whole input is the namespace filter
        const auto namespaceStr = hasNamespaceSep
            ? input.substr(0, namespaceSepPos)
            : input;
        if (!namespaceStr.contains(INVALID_NAMESPACE_FILTER_CHARS)) {
            for (auto t : SplitStringView(namespaceStr, NAMESPACE_SEP)) {
                m_NamespaceTokens.emplace_back(notsa::trim_string(t));
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

        m_IsSimpleGlobalFilter = !hasNamespaceSep && m_NamespaceTokens.size() == 1 && IsHookFilterActive();
    }

    // In case user passes in a string with multiple `/` with nothing in-between we will have quite a few empty tokens.
    // We have have to remove all the leading empty tokens up until the last empty one.
    while (m_NamespaceTokens.size() >= 2 && m_NamespaceTokens[0].empty() && m_NamespaceTokens[1].empty()) {
        m_NamespaceTokens.erase(m_NamespaceTokens.begin());
    }
}


bool HookFilter::IsHookFilterActive() const noexcept { 
    return IsHookFilterPresent() && !m_HookFilter->empty(); 
}

float HookFilter::MatchItem(
    std::string_view name,
    void*            addressA,
    void*            addressB
) const noexcept {
    auto matches = false;
    if (m_HookFilterByName) {
        matches |= StringContainsString(name, *m_HookFilter, m_IsCaseSensitive);
    }
    if (m_HookFilterByAddress) {
        const auto CheckContainsAddress = [&] (void* addr) {
            char buf[64]{ "0x" };
            const auto [end, ec] = std::to_chars(buf + 2, buf + sizeof(buf) - 2, (uintptr_t)(addr), 16);
            if (ec != std::errc{}) {
                return false;
            }
            return StringContainsString(std::string_view{ buf, end }, *m_HookFilter, false);
        };
        matches |= CheckContainsAddress(addressA) || CheckContainsAddress(addressB);
    }
    return matches
        ? 1.0f
        : 0.f;
}

bool HookFilter::IsNamespaceFilterActive() const noexcept {
    if (m_NamespaceTokens.empty()) {
        return false; // No tokens, no filter
    }
    if (IsRootRelativeNamespace() && m_NamespaceTokens.size() == 1) {
        return false; // Only the root namespace is specified, so we're effectively not filtering anything, eg.: input was `/`, `/::`, `::`
    }
    return true;
}

float HookFilter::MatchCategoryByNamespace(
    const NamespaceTokens& path,
    size_t               depth
) const noexcept {
    NOTSA_UNREACHABLE("todo");
    //const auto Match = [&](size_t nsTokenIdx) {
    //    return StringContainsString(name, m_NamespaceTokens[nsTokenIdx], m_IsCaseSensitive)
    //        ? 1.0f
    //        : 0.f;
    //};
    //if (IsRootRelativeNamespace()) {
    //    if (depth == 0) {
    //        return 1.f; // Root always matches
    //    }
    //    if (depth < m_NamespaceTokens.size()) {
    //        return 0.f; // Not enough tokens to match
    //    }
    //    return StringContainsString(path[depth], m_NamespaceTokens[depth], m_IsCaseSensitive)
    //        ? 1.0f
    //        : 0.f;
    //}
    //return Match(std::min(depth, m_NamespaceTokens.size() - 1));

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
    assert(m_NamespaceTokens.size() == 1);
    return StringContainsString(name, m_NamespaceTokens.back(), m_IsCaseSensitive)
        ? 1.0f
        : 0.f;
}
