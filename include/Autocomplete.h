#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

#include "Suggester.h"
#include "Trie.h"

/**
 * @class Autocomplete
 * @brief Concrete suggester implementing prefix-based word completion using a Trie.
 *
 * Demonstrates:
 * - Inheritance: Inherits from Suggester abstract base class.
 * - Polymorphism: Overrides suggest() to return up to 10 prefix-matched words.
 * - Encapsulation: Holds a const reference to the underlying Trie data structure.
 */
class Autocomplete : public Suggester {
private:
    const Trie& trie; ///< Reference to the shared prefix Trie
    size_t maxSuggestions;

public:
    /**
     * @brief Constructor accepting a reference to an existing Trie.
     * @param trieRef Shared Trie to query.
     * @param maxCount Maximum number of completions to return (default 10).
     */
    explicit Autocomplete(const Trie& trieRef, size_t maxCount = 10);

    ~Autocomplete() override = default;

    /**
     * @brief Generates up to 10 words starting with the provided prefix.
     * @param query The prefix to search for.
     * @return Vector of words starting with query prefix.
     */
    std::vector<std::string> suggest(const std::string& query) override;
};

#endif // AUTOCOMPLETE_H

