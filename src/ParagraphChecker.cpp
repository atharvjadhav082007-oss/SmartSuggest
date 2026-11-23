#include "ParagraphChecker.h"
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <unordered_set>

ParagraphChecker::ParagraphChecker(const SpellChecker& checker, size_t maxCharsLimit)
    : spellChecker(checker), maxChars(maxCharsLimit) {}

static bool isTokenChar(unsigned char c) {
    return std::isalnum(c) || c == '\'';
}

std::vector<std::string> ParagraphChecker::tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string current;
    for (char ch : text) {
        if (isTokenChar(static_cast<unsigned char>(ch))) {
            current.push_back(ch);
        } else if (!current.empty()) {
            tokens.push_back(current);
            current.clear();
        }
    }
    if (!current.empty()) {
        tokens.push_back(current);
    }
    return tokens;
}

static bool isCheckable(const std::string& token) {
    for (char ch : token) {
        if (!std::isalpha(static_cast<unsigned char>(ch))) {
            return false; // digit or apostrophe -> skip
        }
    }
    return !token.empty();
}

static std::string lower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

ParagraphChecker::Report ParagraphChecker::analyze(const std::string& paragraph) const {
    Report report;
    std::string text = paragraph;
    if (text.size() > maxChars) {
        text.resize(maxChars);
        report.truncated = true;
    }

    // Each distinct word is checked once, repeats reuse the cached answer.
    std::unordered_map<std::string, WordResult> cache;

    for (const std::string& token : tokenize(text)) {
        ++report.totalWords;

        WordResult result;
        result.word = token;

        if (!isCheckable(token)) {
            result.status = Status::Skipped;
        } else {
            ++report.checkedWords;
            const std::string key = lower(token);
            auto it = cache.find(key);
            if (it == cache.end()) {
                WordResult fresh;
                if (spellChecker.isCorrect(key)) {
                    fresh.status = Status::Correct;
                } else {
                    fresh.status = Status::Misspelled;
                    fresh.suggestions = spellChecker.suggestWithDistance(key);
                }
                it = cache.emplace(key, std::move(fresh)).first;
            }
            result.status = it->second.status;
            result.suggestions = it->second.suggestions;
            if (result.status == Status::Misspelled) {
                ++report.misspelledWords;
            }
        }
        report.words.push_back(std::move(result));
    }
    return report;
}

std::string ParagraphChecker::autoCorrect(const std::string& paragraph) const {
    Report report = analyze(paragraph);
    std::string text = paragraph;
    if (text.size() > maxChars) {
        text.resize(maxChars);
    }

    std::string out;
    size_t tokenIndex = 0;
    size_t i = 0;
    while (i < text.size()) {
        if (!isTokenChar(static_cast<unsigned char>(text[i]))) {
            out.push_back(text[i++]);
            continue;
        }
        size_t j = i;
        while (j < text.size() && isTokenChar(static_cast<unsigned char>(text[j]))) {
            ++j;
        }
        std::string token = text.substr(i, j - i);
        const WordResult& r = report.words[tokenIndex++];
        if (r.status == Status::Misspelled && !r.suggestions.empty()) {
            std::string fix = r.suggestions.front().first;
            if (std::isupper(static_cast<unsigned char>(token[0])) && !fix.empty()) {
                fix[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(fix[0])));
            }
            out += fix;
        } else {
            out += token;
        }
        i = j;
    }
    return out;
}

std::vector<std::string> ParagraphChecker::suggest(const std::string& query) {
    Report report = analyze(query);
    std::vector<std::string> misspelled;
    std::unordered_set<std::string> seen;
    for (const auto& w : report.words) {
        if (w.status == Status::Misspelled && seen.insert(lower(w.word)).second) {
            misspelled.push_back(lower(w.word));
        }
    }
    return misspelled;
}
