#include "SpellChecker.h"
#include "Levenshtein.h"
#include <algorithm>
#include <cctype>

SpellChecker::SpellChecker(const Trie& trieRef, const Dictionary& dictRef, int maxDist, size_t maxCount)
    : trie(trieRef), dictionary(dictRef), maxDistance(maxDist), maxSuggestions(maxCount) {}

static std::string sanitize(const std::string& str) {
    std::string clean = str;
    std::transform(clean.begin(), clean.end(), clean.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return clean;
}

bool SpellChecker::isCorrect(const std::string& word) const {
    if (word.empty()) {
        return false;
    }
    std::string clean = sanitize(word);
    return trie.search(clean);
}

std::vector<std::pair<std::string, int>> SpellChecker::suggestWithDistance(const std::string& query) const {
    if (query.empty()) {
        return {};
    }

    std::string clean = sanitize(query);

    // If word is already correct, requirement states: "returns nothing if the word is correct"
    if (trie.search(clean)) {
        return {};
    }

    std::vector<std::pair<std::string, int>> candidates;
    const std::vector<std::string>& words = dictionary.getWords();

    for (const auto& dictWord : words) {
        // Fast length pruning: if length difference > maxDistance, Levenshtein distance must be > maxDistance
        int lenDiff = static_cast<int>(dictWord.size()) - static_cast<int>(clean.size());
        if (lenDiff < -maxDistance || lenDiff > maxDistance) {
            continue;
        }

        int dist = Levenshtein::distance(clean, dictWord, maxDistance);
        if (dist <= maxDistance) {
            candidates.emplace_back(dictWord, dist);
        }
    }

    // Sort by distance ascending; tie-break by alphabetical order
    std::sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b) {
        if (a.second != b.second) {
            return a.second < b.second;
        }
        return a.first < b.first;
    });

    if (candidates.size() > maxSuggestions) {
        candidates.resize(maxSuggestions);
    }

    return candidates;
}

std::vector<std::string> SpellChecker::suggest(const std::string& query) {
    auto pairs = suggestWithDistance(query);
    std::vector<std::string> results;
    results.reserve(pairs.size());

    for (const auto& p : pairs) {
        results.push_back(p.first);
    }
    return results;
}

