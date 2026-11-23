#ifndef SPELLCHECKER_H
#define SPELLCHECKER_H

#include "Suggester.h"
#include "Trie.h"
#include "Dictionary.h"
#include <utility>
#include <vector>

/**
 * @class SpellChecker
 * @brief Concrete suggester providing edit-distance based spell correction.
 *
 * Demonstrates:
 * - Inheritance: Inherits from Suggester.
 * - Polymorphism: Overrides suggest() to check spelling.
 * - Encapsulation: Interacts with Dictionary and Trie without modifying them.
 */
class SpellChecker : public Suggester {
private:
    const Trie& trie;             ///< Fast exact-match lookup
    const Dictionary& dictionary; ///< Word list for edit distance comparison
    int maxDistance;
    size_t maxSuggestions;

public:
    /**
     * @brief Constructs SpellChecker with Trie and Dictionary references.
     * @param trieRef Trie for O(k) exact word existence checking.
     * @param dictRef Dictionary holding all words.
     * @param maxDist Maximum Levenshtein distance allowed (default 2).
     * @param maxCount Maximum number of suggestions to return (default 5).
     */
    SpellChecker(const Trie& trieRef, const Dictionary& dictRef, int maxDist = 2, size_t maxCount = 5);

    ~SpellChecker() override = default;

    /**
     * @brief Checks if a word is spelled correctly in the dictionary.
     */
    bool isCorrect(const std::string& word) const;

    /**
     * @brief Returns nothing if the word is correct; otherwise returns up to 5 dictionary
     *        words with Levenshtein edit distance <= 2, sorted by distance.
     * @param query Word to check.
     * @return Vector of suggested corrections. Empty if query is correct or has no close matches.
     */
    std::vector<std::string> suggest(const std::string& query) override;

    /**
     * @brief Extended suggestion method returning both the word and its edit distance.
     * @param query Word to check.
     * @return Vector of pairs: (suggested word, edit distance).
     */
    std::vector<std::pair<std::string, int>> suggestWithDistance(const std::string& query) const;
};

#endif // SPELLCHECKER_H

