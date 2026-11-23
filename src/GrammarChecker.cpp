#include "GrammarChecker.h"
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <regex>

GrammarChecker::GrammarChecker(const Trie& trieRef) : trie(trieRef) {}

// ==================== HELPERS ====================

bool GrammarChecker::isVowelSound(const std::string& word) {
    if (word.empty()) return false;
    char first = static_cast<char>(std::tolower(static_cast<unsigned char>(word[0])));
    // Simple heuristic: starts with a vowel letter
    // Exception: "university", "united", "unique" etc. start with 'u' but have consonant sound
    // Exception: "hour", "honest" start with 'h' but have vowel sound
    static const std::unordered_set<std::string> consonantU = {
        "university", "united", "unique", "uniform", "union", "unit", "universal",
        "usage", "useful", "user", "usual", "usually", "utility", "utensil"
    };
    static const std::unordered_set<std::string> vowelH = {
        "hour", "honest", "honor", "honour", "heir", "herb"
    };

    std::string lower = toLower(word);
    if (vowelH.count(lower)) return true;
    if (consonantU.count(lower)) return false;

    return first == 'a' || first == 'e' || first == 'i' || first == 'o' || first == 'u';
}

std::string GrammarChecker::toLower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

std::vector<std::pair<std::string, size_t>> GrammarChecker::tokenizeWithPositions(const std::string& text) {
    std::vector<std::pair<std::string, size_t>> tokens;
    size_t i = 0;
    while (i < text.size()) {
        // Skip non-alpha (but keep position)
        while (i < text.size() && !std::isalpha(static_cast<unsigned char>(text[i])) &&
               text[i] != '\'') {
            ++i;
        }
        if (i >= text.size()) break;
        size_t start = i;
        while (i < text.size() && (std::isalpha(static_cast<unsigned char>(text[i])) ||
               text[i] == '\'')) {
            ++i;
        }
        tokens.emplace_back(text.substr(start, i - start), start);
    }
    return tokens;
}

std::vector<std::pair<size_t, size_t>> GrammarChecker::findSentenceBounds(const std::string& text) {
    std::vector<std::pair<size_t, size_t>> sentences;
    size_t start = 0;
    // skip leading whitespace
    while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) ++start;

    for (size_t i = start; i < text.size(); ++i) {
        if (isSentenceEnd(text[i])) {
            sentences.emplace_back(start, i + 1);
            start = i + 1;
            while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) ++start;
        }
    }
    // Remaining text as a sentence
    if (start < text.size()) {
        sentences.emplace_back(start, text.size());
    }
    return sentences;
}

bool GrammarChecker::isSentenceEnd(char c) {
    return c == '.' || c == '!' || c == '?';
}

// ==================== RULE CHECKERS ====================

void GrammarChecker::checkDoubleWords(const std::string& text, std::vector<Issue>& issues) const {
    auto tokens = tokenizeWithPositions(text);
    for (size_t i = 1; i < tokens.size(); ++i) {
        if (toLower(tokens[i].first) == toLower(tokens[i - 1].first)) {
            Issue issue;
            // span from start of first word to end of second
            issue.position = tokens[i - 1].second;
            issue.length = (tokens[i].second + tokens[i].first.size()) - tokens[i - 1].second;
            issue.original = text.substr(issue.position, issue.length);
            issue.suggestion = tokens[i - 1].first;
            issue.rule = "DOUBLE_WORD";
            issue.message = "Repeated word \"" + tokens[i].first + "\". Remove the duplicate.";
            issue.severity = Severity::Error;
            issues.push_back(std::move(issue));
        }
    }
}

void GrammarChecker::checkSubjectVerbAgreement(const std::string& text, std::vector<Issue>& issues) const {
    auto tokens = tokenizeWithPositions(text);

    // Third-person singular subjects
    static const std::unordered_set<std::string> singularSubjects = {
        "he", "she", "it", "this", "that", "everyone", "someone",
        "anyone", "nobody", "everybody", "somebody", "anybody", "each", "every"
    };
    // Plural subjects
    static const std::unordered_set<std::string> pluralSubjects = {
        "they", "we", "you", "i", "these", "those", "people", "children"
    };

    // Base verbs and their s-forms
    static const std::unordered_map<std::string, std::string> baseToSForm = {
        {"go", "goes"}, {"do", "does"}, {"have", "has"}, {"make", "makes"},
        {"take", "takes"}, {"come", "comes"}, {"see", "sees"}, {"know", "knows"},
        {"get", "gets"}, {"give", "gives"}, {"find", "finds"}, {"think", "thinks"},
        {"say", "says"}, {"want", "wants"}, {"need", "needs"}, {"use", "uses"},
        {"work", "works"}, {"try", "tries"}, {"ask", "asks"}, {"seem", "seems"},
        {"feel", "feels"}, {"leave", "leaves"}, {"call", "calls"}, {"run", "runs"},
        {"write", "writes"}, {"read", "reads"}, {"play", "plays"}, {"move", "moves"},
        {"live", "lives"}, {"believe", "believes"}, {"happen", "happens"}, {"walk", "walks"}
    };

    static std::unordered_map<std::string, std::string> sFormToBase;
    static bool reverseMappingBuilt = false;
    if (!reverseMappingBuilt) {
        for (const auto& pair : baseToSForm) {
            sFormToBase[pair.second] = pair.first;
        }
        reverseMappingBuilt = true;
    }

    for (size_t i = 0; i + 1 < tokens.size(); ++i) {
        std::string subj = toLower(tokens[i].first);
        std::string verb = toLower(tokens[i + 1].first);

        // Singular subject + base verb (should be s-form)
        if (singularSubjects.count(subj) && baseToSForm.count(verb)) {
            // Skip "I" - it uses base form
            if (subj == "i") continue;
            Issue issue;
            issue.position = tokens[i + 1].second;
            issue.length = tokens[i + 1].first.size();
            issue.original = tokens[i + 1].first;
            issue.suggestion = baseToSForm.at(verb);
            issue.rule = "SUBJECT_VERB_SINGULAR";
            issue.message = "\"" + tokens[i].first + "\" is singular. Use \"" + issue.suggestion + "\" instead of \"" + tokens[i + 1].first + "\".";
            issue.severity = Severity::Error;
            issues.push_back(std::move(issue));
        }

        // Plural subject + s-form verb (should be base)
        if (pluralSubjects.count(subj) && sFormToBase.count(verb)) {
            // "I has" -> "I have", "They goes" -> "They go"
            Issue issue;
            issue.position = tokens[i + 1].second;
            issue.length = tokens[i + 1].first.size();
            issue.original = tokens[i + 1].first;
            issue.suggestion = sFormToBase.at(verb);
            issue.rule = "SUBJECT_VERB_PLURAL";
            issue.message = "\"" + tokens[i].first + "\" is plural. Use \"" + issue.suggestion + "\" instead of \"" + tokens[i + 1].first + "\".";
            issue.severity = Severity::Error;
            issues.push_back(std::move(issue));
        }
    }
}

void GrammarChecker::checkArticleAgreement(const std::string& text, std::vector<Issue>& issues) const {
    auto tokens = tokenizeWithPositions(text);
    for (size_t i = 0; i + 1 < tokens.size(); ++i) {
        std::string art = toLower(tokens[i].first);
        const std::string& nextWord = tokens[i + 1].first;
        std::string nextLower = toLower(nextWord);

        // Skip if next word is an adjective that could start with vowel/consonant
        // Simple rule: just check the immediately following word
        if (art == "a" && isVowelSound(nextLower)) {
            Issue issue;
            issue.position = tokens[i].second;
            issue.length = tokens[i].first.size();
            issue.original = tokens[i].first;
            issue.suggestion = (std::isupper(static_cast<unsigned char>(tokens[i].first[0])) ? "An" : "an");
            issue.rule = "ARTICLE_A_AN";
            issue.message = "Use \"an\" before \"" + nextWord + "\" (vowel sound).";
            issue.severity = Severity::Error;
            issues.push_back(std::move(issue));
        } else if (art == "an" && !isVowelSound(nextLower)) {
            Issue issue;
            issue.position = tokens[i].second;
            issue.length = tokens[i].first.size();
            issue.original = tokens[i].first;
            issue.suggestion = (std::isupper(static_cast<unsigned char>(tokens[i].first[0])) ? "A" : "a");
            issue.rule = "ARTICLE_AN_A";
            issue.message = "Use \"a\" before \"" + nextWord + "\" (consonant sound).";
            issue.severity = Severity::Error;
            issues.push_back(std::move(issue));
        }
    }
}

void GrammarChecker::checkCapitalization(const std::string& text, std::vector<Issue>& issues) const {
    auto sentences = findSentenceBounds(text);
    for (const auto& bounds : sentences) {
        // Find first alpha character in this sentence
        size_t pos = bounds.first;
        while (pos < bounds.second && !std::isalpha(static_cast<unsigned char>(text[pos]))) ++pos;
        if (pos < bounds.second && std::islower(static_cast<unsigned char>(text[pos]))) {
            // Check it's not "i" standalone handled elsewhere
            Issue issue;
            issue.position = pos;
            issue.length = 1;
            issue.original = std::string(1, text[pos]);
            issue.suggestion = std::string(1, static_cast<char>(std::toupper(static_cast<unsigned char>(text[pos]))));
            issue.rule = "CAPITALIZATION";
            issue.message = "Sentences should start with a capital letter.";
            issue.severity = Severity::Warning;
            issues.push_back(std::move(issue));
        }
    }

    // Check standalone "i" should be "I"
    auto tokens = tokenizeWithPositions(text);
    for (const auto& tok : tokens) {
        if (tok.first == "i") {
            Issue issue;
            issue.position = tok.second;
            issue.length = 1;
            issue.original = "i";
            issue.suggestion = "I";
            issue.rule = "CAPITALIZE_I";
            issue.message = "The pronoun \"I\" should always be capitalized.";
            issue.severity = Severity::Error;
            issues.push_back(std::move(issue));
        }
    }
}

void GrammarChecker::checkCommonMistakes(const std::string& text, std::vector<Issue>& issues) const {
    // Pattern: multi-word mistakes
    struct MistakePattern {
        std::string wrong;
        std::string correct;
        std::string msg;
        Severity sev;
    };

    static const std::vector<MistakePattern> patterns = {
        {"could of", "could have", "\"could of\" is incorrect. Use \"could have\" (or \"could've\").", Severity::Error},
        {"would of", "would have", "\"would of\" is incorrect. Use \"would have\" (or \"would've\").", Severity::Error},
        {"should of", "should have", "\"should of\" is incorrect. Use \"should have\" (or \"should've\").", Severity::Error},
        {"must of", "must have", "\"must of\" is incorrect. Use \"must have\".", Severity::Error},
        {"might of", "might have", "\"might of\" is incorrect. Use \"might have\".", Severity::Error},
        {"alot", "a lot", "\"alot\" is not a word. Use \"a lot\".", Severity::Error},
        {"irregardless", "regardless", "\"irregardless\" is non-standard. Use \"regardless\".", Severity::Warning},
        {"supposably", "supposedly", "\"supposably\" is non-standard. Use \"supposedly\".", Severity::Warning},
        {"should ought", "should", "Double modal: use \"should\" or \"ought to\", not both.", Severity::Error},
        {"more better", "better", "\"More better\" is a double comparative. Just use \"better\".", Severity::Error},
        {"most biggest", "biggest", "\"Most biggest\" is a double superlative. Just use \"biggest\".", Severity::Error},
        {"can't hardly", "can hardly", "\"Can't hardly\" is a double negative. Use \"can hardly\".", Severity::Error},
        {"don't never", "don't ever", "\"Don't never\" is a double negative. Use \"don't ever\" or \"never\".", Severity::Error},
        {"ain't", "isn't", "\"Ain't\" is non-standard. Use \"isn't\", \"aren't\", or \"haven't\".", Severity::Style},
        {"gonna", "going to", "\"Gonna\" is informal. Use \"going to\" in formal writing.", Severity::Style},
        {"wanna", "want to", "\"Wanna\" is informal. Use \"want to\" in formal writing.", Severity::Style},
        {"gotta", "got to", "\"Gotta\" is informal. Use \"got to\" or \"have to\" in formal writing.", Severity::Style},
    };

    std::string lower = toLower(text);
    for (const auto& pat : patterns) {
        size_t pos = 0;
        while ((pos = lower.find(pat.wrong, pos)) != std::string::npos) {
            // Ensure word boundaries
            bool leftOk = (pos == 0 || !std::isalpha(static_cast<unsigned char>(text[pos - 1])));
            size_t end = pos + pat.wrong.size();
            bool rightOk = (end >= text.size() || !std::isalpha(static_cast<unsigned char>(text[end])));

            if (leftOk && rightOk) {
                Issue issue;
                issue.position = pos;
                issue.length = pat.wrong.size();
                issue.original = text.substr(pos, pat.wrong.size());
                issue.suggestion = pat.correct;
                issue.rule = "COMMON_MISTAKE";
                issue.message = pat.msg;
                issue.severity = pat.sev;
                issues.push_back(std::move(issue));
            }
            pos = end;
        }
    }
}

void GrammarChecker::checkPronounCase(const std::string& text, std::vector<Issue>& issues) const {
    // Subject position pronoun mistakes: "me went" -> "I went", "him went" -> "he went"
    // These only apply at start of sentence or after conjunction
    auto tokens = tokenizeWithPositions(text);
    auto sentences = findSentenceBounds(text);

    static const std::unordered_map<std::string, std::string> objectToSubject = {
        {"me", "I"}, {"him", "he"}, {"her", "she"}, {"them", "they"}, {"us", "we"}
    };

    // Check first word of each sentence
    for (const auto& bounds : sentences) {
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (tokens[i].second >= bounds.first && tokens[i].second < bounds.second) {
                std::string lower = toLower(tokens[i].first);
                // Only if it's the first token of this sentence
                bool isFirst = true;
                for (size_t j = 0; j < i; ++j) {
                    if (tokens[j].second >= bounds.first && tokens[j].second < bounds.second) {
                        isFirst = false;
                        break;
                    }
                }
                if (isFirst && objectToSubject.count(lower) && i + 1 < tokens.size()) {
                    // Check if next word looks like a verb
                    std::string nextLower = toLower(tokens[i + 1].first);
                    static const std::unordered_set<std::string> commonVerbs = {
                        "went", "go", "goes", "run", "runs", "ran", "walked", "walk",
                        "think", "thought", "said", "say", "want", "wanted", "need",
                        "needed", "like", "liked", "am", "is", "are", "was", "were",
                        "have", "has", "had", "do", "does", "did", "can", "will",
                        "would", "should", "could", "shall", "must", "might"
                    };
                    if (commonVerbs.count(nextLower)) {
                        Issue issue;
                        issue.position = tokens[i].second;
                        issue.length = tokens[i].first.size();
                        issue.original = tokens[i].first;
                        const std::string& fix = objectToSubject.at(lower);
                        // Match case of original
                        if (std::isupper(static_cast<unsigned char>(tokens[i].first[0]))) {
                            issue.suggestion = fix; // "I" / "He" etc. already capitalized in map
                        } else {
                            issue.suggestion = toLower(fix);
                        }
                        // Capitalize since it's start of sentence
                        if (!issue.suggestion.empty()) {
                            issue.suggestion[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(issue.suggestion[0])));
                        }
                        issue.rule = "PRONOUN_CASE";
                        issue.message = "\"" + tokens[i].first + "\" is an object pronoun. Use \"" + issue.suggestion + "\" as the subject.";
                        issue.severity = Severity::Error;
                        issues.push_back(std::move(issue));
                    }
                }
                break; // only check first token of each sentence
            }
        }
    }
}

void GrammarChecker::checkEndPunctuation(const std::string& text, std::vector<Issue>& issues) const {
    if (text.empty()) return;
    // Find last non-whitespace character
    size_t last = text.find_last_not_of(" \t\r\n");
    if (last == std::string::npos) return;

    if (!isSentenceEnd(text[last]) && std::isalpha(static_cast<unsigned char>(text[last]))) {
        Issue issue;
        issue.position = last + 1;
        issue.length = 0;
        issue.original = "";
        issue.suggestion = ".";
        issue.rule = "MISSING_PERIOD";
        issue.message = "Sentence appears to be missing ending punctuation.";
        issue.severity = Severity::Warning;
        issues.push_back(std::move(issue));
    }
}

// ==================== MAIN ANALYSIS ====================

std::string GrammarChecker::applySuggestions(const std::string& text, const std::vector<Issue>& issues) const {
    if (issues.empty()) return text;

    // Sort by position descending so we can replace from end to start
    std::vector<const Issue*> sorted;
    for (const auto& issue : issues) {
        sorted.push_back(&issue);
    }
    std::sort(sorted.begin(), sorted.end(), [](const Issue* a, const Issue* b) {
        return a->position > b->position;
    });

    std::string result = text;
    for (const Issue* issue : sorted) {
        if (issue->position <= result.size()) {
            result.replace(issue->position, issue->length, issue->suggestion);
        }
    }
    return result;
}

GrammarChecker::Report GrammarChecker::analyze(const std::string& text) const {
    Report report;
    if (text.empty()) return report;

    auto tokens = tokenizeWithPositions(text);
    report.totalWords = tokens.size();
    report.sentenceCount = findSentenceBounds(text).size();

    // Run all grammar rules
    checkDoubleWords(text, report.issues);
    checkSubjectVerbAgreement(text, report.issues);
    checkArticleAgreement(text, report.issues);
    checkCapitalization(text, report.issues);
    checkCommonMistakes(text, report.issues);
    checkPronounCase(text, report.issues);
    checkEndPunctuation(text, report.issues);

    // Sort issues by position
    std::sort(report.issues.begin(), report.issues.end(),
              [](const Issue& a, const Issue& b) { return a.position < b.position; });

    // Remove overlapping issues (keep highest severity)
    std::vector<Issue> filtered;
    for (auto& issue : report.issues) {
        bool overlaps = false;
        for (const auto& existing : filtered) {
            size_t eEnd = existing.position + existing.length;
            size_t iEnd = issue.position + issue.length;
            if (issue.position < eEnd && iEnd > existing.position) {
                overlaps = true;
                break;
            }
        }
        if (!overlaps) {
            filtered.push_back(std::move(issue));
        }
    }
    report.issues = std::move(filtered);

    // Build corrected text
    report.corrected = applySuggestions(text, report.issues);

    return report;
}

std::vector<std::string> GrammarChecker::suggest(const std::string& query) {
    Report report = analyze(query);
    std::vector<std::string> descriptions;
    for (const auto& issue : report.issues) {
        descriptions.push_back("[" + issue.rule + "] " + issue.message);
    }
    return descriptions;
}
