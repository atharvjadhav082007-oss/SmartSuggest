#ifndef PARAGRAPHCHECKER_H
#define PARAGRAPHCHECKER_H

#include "Suggester.h"
#include "SpellChecker.h"
#include <string>
#include <utility>
#include <vector>

/**
 * @class ParagraphChecker
 * @brief Checks a whole paragraph (many words) by tokenizing it and delegating
 *        every word to a SpellChecker.
 *
 * Demonstrates:
 * - Inheritance / Polymorphism: derives from Suggester, so it can be used
 *   through a Suggester* exactly like Autocomplete and SpellChecker.
 *   suggest(paragraph) returns the list of distinct misspelled words.
 * - Composition: HAS-A SpellChecker (does not re-implement edit distance).
 * - Encapsulation: tokenization rules are private.
 *
 * Tokenization rule (mirrored by public/app.js so the UI lines up index by index):
 *   a token is a maximal run of [A-Za-z0-9'].
 *   Tokens containing a digit or an apostrophe ("don't", "pro123") are SKIPPED
 *   (reported, but never flagged as misspelled).
 */
class ParagraphChecker : public Suggester {
public:
    enum class Status { Correct, Misspelled, Skipped };

    struct WordResult {
        std::string word;                                      ///< Token exactly as typed
        Status status;
        std::vector<std::pair<std::string, int>> suggestions;  ///< (word, edit distance); only for Misspelled
    };

    struct Report {
        std::vector<WordResult> words;   ///< One entry per token, in paragraph order
        size_t totalWords = 0;           ///< All tokens
        size_t checkedWords = 0;         ///< Tokens that were actually looked up
        size_t misspelledWords = 0;      ///< Tokens flagged as misspelled (counting repeats)
        bool truncated = false;          ///< true if the paragraph exceeded maxChars and was cut
    };

private:
    const SpellChecker& spellChecker;
    size_t maxChars;

    static std::vector<std::string> tokenize(const std::string& text);

public:
    /**
     * @param checker  SpellChecker used for every word.
     * @param maxChars Longest paragraph accepted (extra characters are ignored). Default 5000.
     */
    explicit ParagraphChecker(const SpellChecker& checker, size_t maxChars = 5000);
    ~ParagraphChecker() override = default;

    /// Full per-word analysis of a paragraph.
    Report analyze(const std::string& paragraph) const;

    /// Paragraph with every misspelled word replaced by its top suggestion
    /// (words with no suggestion are left untouched; original letter case of the first letter is kept).
    std::string autoCorrect(const std::string& paragraph) const;

    /// Polymorphic entry point: returns the DISTINCT misspelled words, in order of first appearance.
    std::vector<std::string> suggest(const std::string& query) override;
};

#endif // PARAGRAPHCHECKER_H
