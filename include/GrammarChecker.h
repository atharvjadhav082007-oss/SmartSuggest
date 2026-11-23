#ifndef GRAMMARCHECKER_H
#define GRAMMARCHECKER_H

#include "Suggester.h"
#include "Trie.h"
#include <string>
#include <vector>
#include <utility>

/**
 * @class GrammarChecker
 * @brief Concrete suggester providing rule-based English grammar checking.
 *
 * Demonstrates:
 * - Inheritance: Inherits from Suggester.
 * - Polymorphism: Overrides suggest() to detect grammar issues in text.
 * - Encapsulation: Grammar rules, pattern matching, and correction logic are private.
 * - Composition: HAS-A Trie for word existence verification.
 *
 * Grammar rules implemented (pure C++, no external libraries):
 *  1. Subject-verb agreement: "he go" -> "he goes", "they goes" -> "they go"
 *  2. Article-noun agreement: "a apple" -> "an apple", "an car" -> "a car"
 *  3. Double word detection: "the the" -> "the"
 *  4. Capitalization: first word of sentence should be capitalized
 *  5. Missing punctuation at end of sentence
 *  6. Common grammar mistakes: "could of" -> "could have", etc.
 *  7. Pronoun case: "me went" -> "I went", etc.
 *  8. Double negatives: "don't never" -> "don't ever"
 *  9. Its vs It's confusion detection
 * 10. Their/There/They're basic checks
 */
class GrammarChecker : public Suggester {
public:
    enum class Severity { Error, Warning, Style };

    struct Issue {
        size_t position;           ///< Character offset in original text
        size_t length;             ///< Length of the problematic span
        std::string original;      ///< The problematic text
        std::string suggestion;    ///< Suggested replacement
        std::string rule;          ///< Short rule identifier
        std::string message;       ///< Human-readable explanation
        Severity severity;
    };

    struct Report {
        std::vector<Issue> issues;
        size_t totalWords = 0;
        size_t sentenceCount = 0;
        std::string corrected;     ///< Text with all auto-corrections applied
    };

private:
    const Trie& trie;

    // ---- Internal rule checkers ----
    void checkDoubleWords(const std::string& text, std::vector<Issue>& issues) const;
    void checkSubjectVerbAgreement(const std::string& text, std::vector<Issue>& issues) const;
    void checkArticleAgreement(const std::string& text, std::vector<Issue>& issues) const;
    void checkCapitalization(const std::string& text, std::vector<Issue>& issues) const;
    void checkCommonMistakes(const std::string& text, std::vector<Issue>& issues) const;
    void checkPronounCase(const std::string& text, std::vector<Issue>& issues) const;
    void checkEndPunctuation(const std::string& text, std::vector<Issue>& issues) const;

    // Helpers
    static bool isVowelSound(const std::string& word);
    static std::string toLower(const std::string& s);
    static std::vector<std::pair<std::string, size_t>> tokenizeWithPositions(const std::string& text);
    static std::vector<std::pair<size_t, size_t>> findSentenceBounds(const std::string& text);
    static bool isSentenceEnd(char c);
    std::string applySuggestions(const std::string& text, const std::vector<Issue>& issues) const;

public:
    explicit GrammarChecker(const Trie& trieRef);
    ~GrammarChecker() override = default;

    /// Full grammar analysis of a text passage.
    Report analyze(const std::string& text) const;

    /// Polymorphic entry point: returns human-readable issue descriptions.
    std::vector<std::string> suggest(const std::string& query) override;
};

#endif // GRAMMARCHECKER_H
