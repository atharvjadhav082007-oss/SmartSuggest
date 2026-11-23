#include "Dictionary.h"
#include "Trie.h"
#include "Suggester.h"
#include "Autocomplete.h"
#include "SpellChecker.h"
#include "ParagraphChecker.h"
#include "SpellingGame.h"
#include "GrammarChecker.h"
#include "WhatsAppBot.h"
#include <iostream>
#include <memory>
#include <string>
#include <algorithm>
#include <cctype>

// Forward declaration from server.cpp
void startServer(int port = 8080, const std::string& dictPath = "words.txt", const std::string& publicDir = "./public");

/**
 * @brief Validates that the input is non-empty and consists purely of alphabet letters.
 * @param input The raw input string.
 * @param errorMsg Output error explanation if invalid.
 * @return true if valid, false otherwise.
 */
bool validateInput(const std::string& input, std::string& errorMsg) {
    if (input.empty()) {
        errorMsg = "Error: Input cannot be empty.";
        return false;
    }

    for (char c : input) {
        if (!std::isalpha(static_cast<unsigned char>(c))) {
            errorMsg = "Error: Input contains non-letter characters. Only 'a-z' and 'A-Z' are allowed.";
            return false;
        }
    }
    return true;
}

/**
 * @brief Demonstrates RUNTIME POLYMORPHISM (OOP Concept #4).
 *
 * This function accepts a pointer to the abstract base class Suggester*.
 * It invokes the pure virtual method suggest(), which dynamically dispatches
 * to either Autocomplete, SpellChecker, ParagraphChecker, or SpellingGame at runtime.
 */
void runSuggesterPolymorphically(Suggester* suggester, const std::string& query, const std::string& engineName) {
    std::cout << "\n>>> [OOP POLYMORPHISM] Calling virtual suggest() via base pointer (Suggester*) for " << engineName << "...\n";
    std::vector<std::string> results = suggester->suggest(query);

    std::cout << "Results for \"" << query << "\": ";
    if (results.empty()) {
        std::cout << "No suggestions (word is either correct or no close matches)." << std::endl;
    } else {
        std::cout << "(" << results.size() << " match" << (results.size() > 1 ? "es" : "") << ")" << std::endl;
        for (size_t i = 0; i < results.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << results[i] << "\n";
        }
    }
}

void printBanner() {
    std::cout << "===============================================================\n";
    std::cout << "   C++ OOP Autocomplete, Spell Checker & WhatsApp Suite (C++17) \n";
    std::cout << "===============================================================\n";
    std::cout << "Demonstrating Core OOP Concepts:\n";
    std::cout << " [1] Encapsulation   : Dictionary, Trie, WhatsAppBot, SpellingGame\n";
    std::cout << " [2] Abstraction     : Suggester defines pure virtual contract\n";
    std::cout << " [3] Inheritance     : Autocomplete, SpellChecker, ParagraphChecker, SpellingGame derive Suggester\n";
    std::cout << " [4] Polymorphism    : Dynamic dispatch via Suggester* pointer\n";
    std::cout << " [5] Composition     : ParagraphChecker HAS-A SpellChecker; WhatsAppBot HAS-A Suggesters\n";
    std::cout << " [6] RAII / Memory   : TrieNode recursive destructor, copy disabled\n";
    std::cout << "===============================================================\n\n";
}

int main(int argc, char* argv[]) {
    // If "--server" flag passed, launch HTTP server directly
    if (argc > 1 && (std::string(argv[1]) == "--server" || std::string(argv[1]) == "-s")) {
        int port = 8080;
        const char* portEnv = std::getenv("PORT");
        if (portEnv) {
            try {
                port = std::stoi(portEnv);
            } catch (...) {}
        }
        if (argc > 2) {
            try {
                port = std::stoi(argv[2]);
            } catch (...) {}
        }
        startServer(port);
        return 0;
    }

    printBanner();

    // 1. Encapsulation: Instantiate and load Dictionary
    const std::string dictPath = "words.txt";
    std::cout << "[Init] Loading dictionary from '" << dictPath << "'..." << std::endl;
    Dictionary dictionary(dictPath);

    if (dictionary.size() == 0) {
        std::cerr << "[Warning] Could not load '" << dictPath << "' in current folder. Trying '../words.txt'..." << std::endl;
        dictionary.loadFromFile("../" + dictPath);
    }

    std::cout << "[Init] Dictionary loaded " << dictionary.size() << " words successfully.\n";

    // 2. Data Structure: Build Trie with RAII
    Trie trie;
    for (const auto& w : dictionary.getWords()) {
        trie.insert(w);
    }
    std::cout << "[Init] Prefix Trie populated successfully.\n\n";

    // 3. Inheritance & Abstraction: Instantiate concrete suggesters
    std::unique_ptr<Suggester> autocompleteSuggester = std::make_unique<Autocomplete>(trie, 10);
    std::unique_ptr<SpellChecker> spellChecker = std::make_unique<SpellChecker>(trie, dictionary, 2, 5);
    std::unique_ptr<ParagraphChecker> paragraphChecker = std::make_unique<ParagraphChecker>(*spellChecker, 5000);
    std::unique_ptr<SpellingGame> spellingGame = std::make_unique<SpellingGame>(dictionary);
    std::unique_ptr<GrammarChecker> grammarChecker = std::make_unique<GrammarChecker>(trie);

    // Interactive Console Menu
    while (true) {
        std::cout << "\n------------------- MENU -------------------\n";
        std::cout << " 1. Autocomplete (Prefix Search, up to 10 words)\n";
        std::cout << " 2. Spell Check  (Levenshtein Distance <= 2, up to 5 words)\n";
        std::cout << " 3. Paragraph Spell Check (check every word in a paragraph)\n";
        std::cout << " 4. Grammar Checker (Rule-based grammar and style checks)\n";
        std::cout << " 5. Spelling Challenge Game (Interactive CLI mode)\n";
        std::cout << " 6. Start Web Server Mode (HTTP + WhatsApp Webhook on port 8080)\n";
        std::cout << " 7. Exit\n";
        std::cout << "--------------------------------------------\n";
        std::cout << "Choose an option (1-7): ";

        std::string choice;
        if (!std::getline(std::cin, choice)) {
            break;
        }

        // Trim choice
        size_t cFirst = choice.find_first_not_of(" \t\r\n");
        if (cFirst == std::string::npos) continue;
        choice = choice.substr(cFirst);

        if (choice == "1") {
            std::cout << "\n[Mode: Autocomplete]\nEnter prefix to search: ";
            std::string input;
            if (!std::getline(std::cin, input)) break;

            size_t start = input.find_first_not_of(" \t\r\n");
            size_t end = input.find_last_not_of(" \t\r\n");
            input = (start == std::string::npos) ? "" : input.substr(start, end - start + 1);

            std::string errorMsg;
            if (!validateInput(input, errorMsg)) {
                std::cout << errorMsg << "\n";
                continue;
            }

            runSuggesterPolymorphically(autocompleteSuggester.get(), input, "Autocomplete");

        } else if (choice == "2") {
            std::cout << "\n[Mode: Spell Check]\nEnter a word to verify: ";
            std::string input;
            if (!std::getline(std::cin, input)) break;

            size_t start = input.find_first_not_of(" \t\r\n");
            size_t end = input.find_last_not_of(" \t\r\n");
            input = (start == std::string::npos) ? "" : input.substr(start, end - start + 1);

            std::string errorMsg;
            if (!validateInput(input, errorMsg)) {
                std::cout << errorMsg << "\n";
                continue;
            }

            if (spellChecker->isCorrect(input)) {
                std::cout << "\n✓ Correct! \"" << input << "\" is spelled correctly according to the dictionary." << std::endl;
                std::vector<std::string> none = spellChecker->suggest(input);
                std::cout << "  (Polymorphic suggest() returned " << none.size() << " suggestions as expected.)" << std::endl;
            } else {
                std::cout << "\n✗ Misspelled word: \"" << input << "\"\n";
                auto detailed = spellChecker->suggestWithDistance(input);
                if (detailed.empty()) {
                    std::cout << "No dictionary words found within edit distance of 2." << std::endl;
                } else {
                    std::cout << "Did you mean:\n";
                    for (size_t i = 0; i < detailed.size(); ++i) {
                        std::cout << "  " << (i + 1) << ". " << detailed[i].first
                                  << " (Levenshtein distance: " << detailed[i].second << ")\n";
                    }
                }
                runSuggesterPolymorphically(spellChecker.get(), input, "SpellChecker");
            }

        } else if (choice == "3") {
            std::cout << "\n[Mode: Paragraph Spell Check]\nPaste a paragraph (one line, max 5000 chars): ";
            std::string input;
            if (!std::getline(std::cin, input)) break;

            size_t start = input.find_first_not_of(" \t\r\n");
            size_t end = input.find_last_not_of(" \t\r\n");
            input = (start == std::string::npos) ? "" : input.substr(start, end - start + 1);

            if (input.empty()) {
                std::cout << "Error: Paragraph cannot be empty.\n";
                continue;
            }

            ParagraphChecker::Report report = paragraphChecker->analyze(input);
            std::cout << "\nWords: " << report.totalWords
                      << " | Checked: " << report.checkedWords
                      << " | Misspelled: " << report.misspelledWords << "\n";
            if (report.truncated) {
                std::cout << "(Paragraph was longer than 5000 characters and was truncated.)\n";
            }

            if (report.misspelledWords == 0) {
                std::cout << "\n✓ No spelling mistakes found.\n";
            } else {
                std::cout << "\nMisspelled words:\n";
                std::vector<std::string> shown;
                for (const auto& w : report.words) {
                    if (w.status != ParagraphChecker::Status::Misspelled) continue;
                    if (std::find(shown.begin(), shown.end(), w.word) != shown.end()) continue;
                    shown.push_back(w.word);
                    std::cout << "  ✗ " << w.word << " -> ";
                    if (w.suggestions.empty()) {
                        std::cout << "(no close matches)";
                    } else {
                        for (size_t i = 0; i < w.suggestions.size(); ++i) {
                            if (i > 0) std::cout << ", ";
                            std::cout << w.suggestions[i].first << " (" << w.suggestions[i].second << ")";
                        }
                    }
                    std::cout << "\n";
                }
                std::cout << "\nAuto-corrected (top suggestion):\n  " << paragraphChecker->autoCorrect(input) << "\n";
            }

            runSuggesterPolymorphically(paragraphChecker.get(), input, "ParagraphChecker (returns misspelled words)");

        } else if (choice == "4") {
            std::cout << "\n[Mode: Grammar Checker]\nPaste a sentence or paragraph to check grammar: ";
            std::string input;
            if (!std::getline(std::cin, input)) break;

            size_t start = input.find_first_not_of(" \t\r\n");
            size_t end = input.find_last_not_of(" \t\r\n");
            input = (start == std::string::npos) ? "" : input.substr(start, end - start + 1);

            if (input.empty()) {
                std::cout << "Error: Input cannot be empty.\n";
                continue;
            }

            GrammarChecker::Report report = grammarChecker->analyze(input);
            std::cout << "\nWords: " << report.totalWords << " | Sentences: " << report.sentenceCount
                      << " | Issues: " << report.issues.size() << "\n";

            if (report.issues.empty()) {
                std::cout << "\n✓ No grammar issues found!\n";
            } else {
                std::cout << "\nGrammar Issues:\n";
                for (size_t i = 0; i < report.issues.size(); ++i) {
                    const auto& iss = report.issues[i];
                    std::cout << "  " << (i + 1) << ". [" << iss.rule << "] " << iss.message;
                    if (!iss.suggestion.empty()) {
                        std::cout << " -> Suggestion: \"" << iss.suggestion << "\"";
                    }
                    std::cout << "\n";
                }
                std::cout << "\nAuto-corrected text:\n  " << report.corrected << "\n";
            }

            runSuggesterPolymorphically(grammarChecker.get(), input, "GrammarChecker");

        } else if (choice == "5") {
            std::cout << "\n[Mode: Spelling Challenge Game]\n";
            std::cout << "Select difficulty level (1 = Easy, 2 = Medium, 3 = Hard): ";
            std::string lvlStr;
            std::getline(std::cin, lvlStr);
            int lvl = 1;
            try { lvl = std::stoi(lvlStr); } catch (...) { lvl = 1; }

            auto session = spellingGame->startNewGame(lvl);
            std::cout << "\n🎮 Unscramble or correct this corrupted word: [" << session.scrambledWord << "]\n";
            std::cout << "Type your answer: ";
            std::string userAns;
            std::getline(std::cin, userAns);

            auto res = spellingGame->submitAnswer(session.id, userAns);
            std::cout << "\n" << res.message << "\n";
            std::cout << "Target word: " << res.targetWord << " | Distance: " << res.distance << "\n";
            std::cout << "Points earned: " << res.points << " | Current streak: " << res.streak << " | Total score: " << res.totalScore << "\n";

            // Demonstrate polymorphic Suggester on the game
            runSuggesterPolymorphically(spellingGame.get(), session.scrambledWord, "SpellingGame");

        } else if (choice == "6") {
            std::cout << "\nStarting Web Server..." << std::endl;
            startServer(8080);
            break;

        } else if (choice == "7" || choice == "exit" || choice == "q") {
            std::cout << "Exiting application. Goodbye!\n";
            break;

        } else {
            std::cout << "Invalid choice! Please select an option between 1 and 7.\n";
        }
    }

    return 0;
}
