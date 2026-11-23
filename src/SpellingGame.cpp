#include "SpellingGame.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

SpellingGame::SpellingGame(const Dictionary& dict)
    : dictionary(dict) {
    std::random_device rd;
    rng.seed(rd());
}

std::string SpellingGame::generateSessionId() const {
    static const char chars[] = "0123456789abcdef";
    std::uniform_int_distribution<int> dist(0, 15);
    std::string id = "game_";
    for (int i = 0; i < 12; ++i) {
        id += chars[dist(rng)];
    }
    return id;
}

std::string SpellingGame::corruptWord(const std::string& word, int edits) const {
    if (word.empty()) return word;
    std::string res = word;

    for (int e = 0; e < edits; ++e) {
        // Choose mutation operation:
        // 0: Swap adjacent letters
        // 1: Drop a letter (if length > 3)
        // 2: Duplicate a letter
        // 3: Substitute a letter
        std::vector<int> ops;
        if (res.size() > 1) ops.push_back(0); // swap
        if (res.size() > 3) ops.push_back(1); // drop
        ops.push_back(2);                    // duplicate
        ops.push_back(3);                    // substitute

        std::uniform_int_distribution<size_t> opDist(0, ops.size() - 1);
        int op = ops[opDist(rng)];

        if (op == 0 && res.size() > 1) { // Swap adjacent
            std::uniform_int_distribution<size_t> idxDist(0, res.size() - 2);
            size_t idx = idxDist(rng);
            std::swap(res[idx], res[idx + 1]);
        } else if (op == 1 && res.size() > 3) { // Drop
            std::uniform_int_distribution<size_t> idxDist(0, res.size() - 1);
            size_t idx = idxDist(rng);
            res.erase(res.begin() + idx);
        } else if (op == 2) { // Duplicate
            std::uniform_int_distribution<size_t> idxDist(0, res.size() - 1);
            size_t idx = idxDist(rng);
            res.insert(res.begin() + idx, res[idx]);
        } else { // Substitute
            std::uniform_int_distribution<size_t> idxDist(0, res.size() - 1);
            size_t idx = idxDist(rng);
            std::uniform_int_distribution<int> letterDist(0, 25);
            char newChar = static_cast<char>('a' + letterDist(rng));
            while (newChar == res[idx]) {
                newChar = static_cast<char>('a' + letterDist(rng));
            }
            res[idx] = newChar;
        }
    }

    // If mutation resulted in the same word by accident, guarantee at least 1 swap/substitute
    if (res == word && res.size() > 1) {
        std::swap(res[0], res[1]);
    }
    return res;
}

void SpellingGame::cleanExpiredSessions() {
    auto now = std::chrono::steady_clock::now();
    for (auto it = sessions.begin(); it != sessions.end();) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.lastActive).count();
        if (elapsed > EXPIRY_SECONDS) {
            it = sessions.erase(it);
        } else {
            ++it;
        }
    }
}

SpellingGame::Session SpellingGame::startNewGame(int level) {
    LockGuard lock(sessionMutex);
    cleanExpiredSessions();

    if (level < 1) level = 1;
    if (level > 3) level = 3;

    // Pick a candidate word from dictionary matching level difficulty
    const auto& words = dictionary.getWords();
    std::vector<std::string> candidates;
    for (const auto& w : words) {
        if (level == 1 && w.size() >= 4 && w.size() <= 6) candidates.push_back(w);
        else if (level == 2 && w.size() >= 6 && w.size() <= 9) candidates.push_back(w);
        else if (level == 3 && w.size() >= 8) candidates.push_back(w);
    }
    if (candidates.empty()) candidates = words;

    std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
    std::string target = candidates[dist(rng)];
    std::string scrambled = corruptWord(target, level);

    Session session;
    session.id = generateSessionId();
    session.targetWord = target;
    session.scrambledWord = scrambled;
    session.level = level;
    session.score = 0;
    session.streak = 0;
    session.lastActive = std::chrono::steady_clock::now();

    sessions[session.id] = session;
    return session;
}

int SpellingGame::calculatePoints(int distance) {
    if (distance == 0) return 10;
    if (distance == 1) return 5;
    if (distance == 2) return 2;
    return 0;
}

SpellingGame::AnswerResult SpellingGame::submitAnswer(const std::string& sessionId, const std::string& answer) {
    LockGuard lock(sessionMutex);
    cleanExpiredSessions();

    AnswerResult res;
    res.answer = answer;

    auto it = sessions.find(sessionId);
    if (it == sessions.end()) {
        res.message = "Session expired or not found. Start a new game.";
        return res;
    }

    Session& session = it->second;
    session.lastActive = std::chrono::steady_clock::now();
    res.targetWord = session.targetWord;

    std::string cleanAns = answer;
    std::transform(cleanAns.begin(), cleanAns.end(), cleanAns.begin(), ::tolower);
    // trim
    size_t f = cleanAns.find_first_not_of(" \t\r\n");
    size_t l = cleanAns.find_last_not_of(" \t\r\n");
    if (f != std::string::npos && l != std::string::npos) {
        cleanAns = cleanAns.substr(f, l - f + 1);
    } else {
        cleanAns = "";
    }

    int dist = Levenshtein::distance(cleanAns, session.targetWord);
    res.distance = dist;
    res.correct = (dist == 0);

    int basePts = calculatePoints(dist);
    if (dist <= 1) {
        session.streak++;
        int streakBonus = (session.streak > 1) ? (session.streak * 2) : 0;
        res.points = basePts + streakBonus;
    } else if (dist == 2) {
        res.points = basePts;
    } else {
        session.streak = 0;
        res.points = 0;
    }

    session.score += res.points;
    res.totalScore = session.score;
    res.streak = session.streak;

    if (dist == 0) {
        res.message = "Perfect! Exactly correct!";
    } else if (dist == 1) {
        res.message = "Very close! (Edit distance 1)";
    } else if (dist == 2) {
        res.message = "Close attempt! (Edit distance 2)";
    } else {
        res.message = "Incorrect! The word was: " + session.targetWord;
    }

    return res;
}

bool SpellingGame::hasSession(const std::string& sessionId) {
    LockGuard lock(sessionMutex);
    cleanExpiredSessions();
    return sessions.find(sessionId) != sessions.end();
}

bool SpellingGame::getSession(const std::string& sessionId, Session& outSession) {
    LockGuard lock(sessionMutex);
    cleanExpiredSessions();
    auto it = sessions.find(sessionId);
    if (it != sessions.end()) {
        outSession = it->second;
        return true;
    }
    return false;
}

std::vector<std::string> SpellingGame::suggest(const std::string& query) {
    // Polymorphic implementation: find words matching within distance 2
    std::vector<std::string> result;
    if (query.empty()) return result;

    for (const auto& w : dictionary.getWords()) {
        if (std::abs(static_cast<int>(w.size()) - static_cast<int>(query.size())) > 2) continue;
        int d = Levenshtein::distance(query, w);
        if (d <= 2) {
            result.push_back(w);
            if (result.size() >= 5) break;
        }
    }
    return result;
}
