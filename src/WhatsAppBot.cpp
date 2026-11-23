#include "WhatsAppBot.h"
#include "CryptoUtils.h"
#include <sstream>
#include <algorithm>
#include <cctype>

WhatsAppBot::WhatsAppBot(Autocomplete& ac,
                         SpellChecker& sc,
                         ParagraphChecker& pc,
                         SpellingGame& sg)
    : autocomplete(ac), spellChecker(sc), paragraphChecker(pc), spellingGame(sg) {}

std::string WhatsAppBot::escapeXml(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 16);
    for (char c : s) {
        switch (c) {
            case '&':  out += "&amp;"; break;
            case '<':  out += "&lt;"; break;
            case '>':  out += "&gt;"; break;
            case '"':  out += "&quot;"; break;
            case '\'': out += "&apos;"; break;
            default:   out += c; break;
        }
    }
    return out;
}

std::string WhatsAppBot::toTwiML(const std::string& text) {
    // Keep reply within 1500 characters
    std::string safeText = text;
    if (safeText.size() > 1400) {
        safeText = safeText.substr(0, 1397) + "...";
    }

    std::ostringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml << "<Response>\n";
    xml << "  <Message>" << escapeXml(safeText) << "</Message>\n";
    xml << "</Response>";
    return xml.str();
}

bool WhatsAppBot::checkRateLimit(const std::string& sender) {
    LockGuard lock(botMutex);
    auto now = std::chrono::steady_clock::now();
    auto& rl = rateLimits[sender];

    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - rl.windowStart).count();
    if (elapsed > RATE_LIMIT_WINDOW_SECS || rl.count == 0) {
        rl.windowStart = now;
        rl.count = 1;
        return true;
    }

    if (rl.count >= RATE_LIMIT_MAX) {
        return false;
    }

    rl.count++;
    return true;
}

bool WhatsAppBot::validateTwilioSignature(const std::string& authToken,
                                         const std::string& url,
                                         const std::map<std::string, std::string>& postParams,
                                         const std::string& expectedSignature) {
    if (authToken.empty() || expectedSignature.empty()) return false;

    // Twilio signature construction:
    // 1. Take full request URL
    // 2. Sort all POST parameters alphabetically by key
    // 3. Append each key and its value to the URL string
    // 4. Compute HMAC-SHA1 using authToken as secret key
    // 5. Base64 encode the result and compare
    std::string dataToSign = url;

    // std::map is automatically sorted by key
    for (const auto& pair : postParams) {
        dataToSign += pair.first;
        dataToSign += pair.second;
    }

    auto hmacDigest = CryptoUtils::hmacSha1(authToken, dataToSign);
    std::string calculatedSig = CryptoUtils::base64Encode(hmacDigest);

    return calculatedSig == expectedSignature;
}

std::string WhatsAppBot::reply(const std::string& body) {
    return reply("default_user", body);
}

std::string WhatsAppBot::reply(const std::string& sender, const std::string& body) {
    // Trim input
    size_t f = body.find_first_not_of(" \t\r\n");
    size_t l = body.find_last_not_of(" \t\r\n");
    std::string text = (f != std::string::npos && l != std::string::npos) ? body.substr(f, l - f + 1) : "";

    if (text.empty()) {
        return "👋 Hi! Send *help* to see available commands or type any sentence to spell-check it.";
    }

    // Check if the sender has an active spelling game session
    {
        LockGuard lock(botMutex);
        auto it = senderGameSessions.find(sender);
        if (it != senderGameSessions.end()) {
            std::string sessionId = it->second;
            // Check if user is typing "game" again or "cancel"
            std::string lowerText = text;
            std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::tolower);
            if (lowerText != "game" && lowerText != "help" && lowerText != "cancel") {
                // Treat this message as the answer to the game!
                senderGameSessions.erase(it);
                auto res = spellingGame.submitAnswer(sessionId, text);
                std::ostringstream oss;
                if (res.correct) {
                    oss << "🎉 " << res.message << "\n"
                        << "⭐ Points: +" << res.points << "\n"
                        << "🔥 Streak: " << res.streak << "\n"
                        << "🏆 Total Score: " << res.totalScore << "\n\n"
                        << "Type *game* to play another round!";
                } else {
                    oss << "❌ " << res.message << "\n"
                        << "Your answer: " << res.answer << " (Levenshtein distance: " << res.distance << ")\n"
                        << "⭐ Points: +" << res.points << "\n"
                        << "🏆 Total Score: " << res.totalScore << "\n\n"
                        << "Type *game* to try again!";
                }
                return oss.str();
            } else if (lowerText == "cancel") {
                senderGameSessions.erase(it);
                return "Spelling game cancelled. Send *help* for options.";
            }
        }
    }

    // Parse command vs payload
    std::string cmd;
    std::string payload;
    size_t spacePos = text.find(' ');
    if (spacePos != std::string::npos) {
        cmd = text.substr(0, spacePos);
        size_t pf = text.find_first_not_of(" \t", spacePos);
        if (pf != std::string::npos) payload = text.substr(pf);
    } else {
        cmd = text;
    }

    std::string lowerCmd = cmd;
    std::transform(lowerCmd.begin(), lowerCmd.end(), lowerCmd.begin(), ::tolower);

    // 1. HELP command
    if (lowerCmd == "help") {
        return "🤖 *SmartSuggest WhatsApp Bot Commands:*\n\n"
               "• *check <text>*: Find spelling mistakes & suggestions\n"
               "• *fix <text>*: Auto-correct misspelled words\n"
               "• *complete <prefix>*: Up to 10 autocomplete suggestions\n"
               "• *game*: Start a round of Spelling Challenge\n"
               "• Any normal message is checked automatically!";
    }

    // 2. COMPLETE command
    if (lowerCmd == "complete") {
        if (payload.empty()) {
            return "Please provide a prefix. Example: *complete pro*";
        }
        auto suggestions = autocomplete.suggest(payload);
        if (suggestions.empty()) {
            return "No autocomplete completions found for \"" + payload + "\".";
        }
        std::ostringstream oss;
        oss << "⚡ *Completions for '" << payload << "':*\n";
        for (size_t i = 0; i < suggestions.size(); ++i) {
            oss << (i + 1) << ". " << suggestions[i] << "\n";
        }
        return oss.str();
    }

    // 3. FIX command
    if (lowerCmd == "fix") {
        if (payload.empty()) {
            return "Please provide text to fix. Example: *fix We definately love progamming*";
        }
        std::string corrected = paragraphChecker.autoCorrect(payload);
        return "✨ *Auto-Corrected Text:*\n\n" + corrected;
    }

    // 4. GAME command
    if (lowerCmd == "game") {
        auto session = spellingGame.startNewGame(1);
        {
            LockGuard lock(botMutex);
            senderGameSessions[sender] = session.id;
        }
        std::ostringstream oss;
        oss << "🎮 *Spelling Challenge!*\n\n"
            << "Unscramble/correct this word:\n👉 *" << session.scrambledWord << "*\n\n"
            << "Reply with your answer in your next message! (or send 'cancel')";
        return oss.str();
    }

    // 5. CHECK command or plain text check
    std::string textToCheck = (lowerCmd == "check") ? payload : text;
    if (textToCheck.empty()) {
        return "Please provide text to check. Example: *check definately progam*";
    }

    ParagraphChecker::Report report = paragraphChecker.analyze(textToCheck);
    if (report.misspelledWords == 0) {
        return "✅ *All Good!* No spelling errors found in your text.";
    }

    std::ostringstream oss;
    oss << "🔍 *Spell Check Report:*\n"
        << "• Words analyzed: " << report.checkedWords << "\n"
        << "• Mistakes found: " << report.misspelledWords << "\n\n"
        << "*Suggestions:*\n";

    std::vector<std::string> shown;
    for (const auto& w : report.words) {
        if (w.status != ParagraphChecker::Status::Misspelled) continue;
        if (std::find(shown.begin(), shown.end(), w.word) != shown.end()) continue;
        shown.push_back(w.word);

        oss << "❌ *" << w.word << "* -> ";
        if (w.suggestions.empty()) {
            oss << "no close matches\n";
        } else {
            for (size_t k = 0; k < w.suggestions.size(); ++k) {
                if (k > 0) oss << ", ";
                oss << w.suggestions[k].first << " (d=" << w.suggestions[k].second << ")";
            }
            oss << "\n";
        }
    }

    oss << "\n💡 Send *fix " << textToCheck << "* to auto-correct it.";
    return oss.str();
}
