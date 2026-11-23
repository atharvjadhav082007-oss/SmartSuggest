#ifndef WHATSAPPBOT_H
#define WHATSAPPBOT_H

#include "Autocomplete.h"
#include "SpellChecker.h"
#include "ParagraphChecker.h"
#include "SpellingGame.h"
#include "MutexRAII.h"
#include <string>
#include <vector>
#include <map>
#include <chrono>

/**
 * @class WhatsAppBot
 * @brief WhatsApp webhook chatbot handler implementing conversation flow and Twilio integration.
 * 
 * Demonstrates OOP Concepts:
 * - Composition & Delegation: HAS-A references to Autocomplete, SpellChecker, ParagraphChecker,
 *   and SpellingGame. Delegates all NLP and game commands to their specialized engines.
 * - Encapsulation: Manages per-sender conversation state (active game sessions) and rate-limiting
 *   internally behind a clean interface.
 * - Independence from HTTP: Exposes reply(sender, body) which can be fully unit-tested
 *   without network sockets or HTTP request mocks.
 * - RAII: Uses custom LockGuard for state protection across concurrent requests.
 */
class WhatsAppBot {
public:
    struct RateLimiter {
        int count = 0;
        std::chrono::steady_clock::time_point windowStart;
    };

private:
    Autocomplete& autocomplete;
    SpellChecker& spellChecker;
    ParagraphChecker& paragraphChecker;
    SpellingGame& spellingGame;

    Mutex botMutex;
    // Maps sender -> active game session ID
    std::map<std::string, std::string> senderGameSessions;
    // Maps sender -> rate limiter state
    std::map<std::string, RateLimiter> rateLimits;

    static constexpr int RATE_LIMIT_MAX = 20; // 20 requests
    static constexpr int RATE_LIMIT_WINDOW_SECS = 60; // per minute

public:
    WhatsAppBot(Autocomplete& ac,
                SpellChecker& sc,
                ParagraphChecker& pc,
                SpellingGame& sg);

    /// Main logic entry: processes incoming user text from a given sender phone number
    std::string reply(const std::string& sender, const std::string& body);

    /// Overload for testing without sender context
    std::string reply(const std::string& body);

    /// Checks and updates per-sender rate limit. Returns true if allowed, false if limit exceeded.
    bool checkRateLimit(const std::string& sender);

    /// Wraps text in TwiML XML format: <Response><Message>...</Message></Response>
    static std::string toTwiML(const std::string& text);

    /// Validates Twilio signature header against request parameters using HMAC-SHA1
    static bool validateTwilioSignature(const std::string& authToken,
                                        const std::string& url,
                                        const std::map<std::string, std::string>& postParams,
                                        const std::string& expectedSignature);

    /// Escapes string for safe XML inclusion (&, <, >, ", ')
    static std::string escapeXml(const std::string& s);
};

#endif // WHATSAPPBOT_H
