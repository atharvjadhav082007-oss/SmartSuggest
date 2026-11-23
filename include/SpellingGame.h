#ifndef SPELLINGGAME_H
#define SPELLINGGAME_H

#include "Suggester.h"
#include "Dictionary.h"
#include "Levenshtein.h"
#include "MutexRAII.h"
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <random>

/**
 * @class SpellingGame
 * @brief Interactive spelling challenge game engine with level-based corruption and scoring.
 * 
 * Demonstrates OOP Concepts:
 * - Inheritance / Polymorphism: Inherits from Suggester. Its polymorphic suggest(query)
 *   returns valid dictionary words within edit distance <= 2 for a challenged word.
 * - Composition: HAS-A Dictionary reference and utilizes Levenshtein edit distance.
 * - Encapsulation: Game session state (scores, active challenges, timestamps, corruption logic)
 *   is private and thread-safe via RAII Mutex locking.
 * - RAII: Uses custom LockGuard for thread-safe session synchronization.
 */
class SpellingGame : public Suggester {
public:
    struct Session {
        std::string id;
        std::string targetWord;
        std::string scrambledWord;
        int level = 1;
        int score = 0;
        int streak = 0;
        std::chrono::steady_clock::time_point lastActive;
    };

    struct AnswerResult {
        bool correct = false;
        std::string answer;
        std::string targetWord;
        int distance = 0;
        int points = 0;
        int totalScore = 0;
        int streak = 0;
        std::string message;
    };

private:
    const Dictionary& dictionary;
    mutable std::mt19937 rng;
    mutable Mutex sessionMutex;
    std::map<std::string, Session> sessions;
    static constexpr int EXPIRY_SECONDS = 300; // 5 min session TTL

    std::string generateSessionId() const;
    void cleanExpiredSessions();

public:
    explicit SpellingGame(const Dictionary& dict);
    ~SpellingGame() override = default;

    /// Corrupts a word with a specific number of edits (swap, drop, duplicate, substitute).
    std::string corruptWord(const std::string& word, int edits) const;

    /// Starts a new challenge session for web/API.
    Session startNewGame(int level = 1);

    /// Verifies answer and returns scoring result.
    AnswerResult submitAnswer(const std::string& sessionId, const std::string& answer);

    /// Checks if a session exists and is active.
    bool hasSession(const std::string& sessionId);

    /// Returns session copy if found.
    bool getSession(const std::string& sessionId, Session& outSession);

    /// Static score calculation based on Levenshtein distance: exact=10, dist 1=5, dist 2=2, else 0.
    static int calculatePoints(int distance);

    /// Polymorphic entry point from Suggester:
    /// Suggests valid corrections/candidates from dictionary for a given scrambled/challenge word.
    std::vector<std::string> suggest(const std::string& query) override;
};

#endif // SPELLINGGAME_H
