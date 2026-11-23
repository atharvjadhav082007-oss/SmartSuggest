#include "Autocomplete.h"
#include <algorithm>
#include <cctype>

Autocomplete::Autocomplete(const Trie& trieRef, size_t maxCount)
    : trie(trieRef), maxSuggestions(maxCount) {}

std::vector<std::string> Autocomplete::suggest(const std::string& query) {
    if (query.empty()) {
        return {};
    }

    // Sanitize query to lowercase
    std::string cleanQuery = query;
    std::transform(cleanQuery.begin(), cleanQuery.end(), cleanQuery.begin(), [](unsigned char c) {
        return std::tolower(c);
    });

    return trie.wordsWithPrefix(cleanQuery, maxSuggestions);
}

