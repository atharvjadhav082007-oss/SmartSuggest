#include "httplib.h"
#include "Dictionary.h"
#include "Trie.h"
#include "Autocomplete.h"
#include "SpellChecker.h"
#include "ParagraphChecker.h"
#include "SpellingGame.h"
#include "WhatsAppBot.h"
#include "CryptoUtils.h"
#include "GrammarChecker.h"
#include <iostream>
#include <sstream>
#include <cstdlib>

// Helper to escape JSON string values
static std::string escapeJson(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\b') out += "\\b";
        else if (c == '\f') out += "\\f";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else out += c;
    }
    return out;
}

void startServer(int port = 8080, const std::string& dictPath = "words.txt", const std::string& publicDir = "./public") {
    std::cout << "[Server] Initializing Dictionary and Trie from '" << dictPath << "'..." << std::endl;

    Dictionary dict;
    if (!dict.loadFromFile(dictPath)) {
        std::cerr << "[Server] Warning: Failed to open " << dictPath << ". Trying parent directory..." << std::endl;
        if (!dict.loadFromFile("../" + dictPath)) {
            std::cerr << "[Server] Error: Could not locate dictionary file!" << std::endl;
        }
    }

    Trie trie;
    for (const auto& w : dict.getWords()) {
        trie.insert(w);
    }
    std::cout << "[Server] Loaded " << dict.size() << " words into Trie." << std::endl;

    Autocomplete autocomplete(trie, 10);
    SpellChecker spellChecker(trie, dict, 2, 5);
    ParagraphChecker paragraphChecker(spellChecker, 5000);
    SpellingGame spellingGame(dict);
    GrammarChecker grammarChecker(trie);
    WhatsAppBot whatsAppBot(autocomplete, spellChecker, paragraphChecker, spellingGame);

    const char* twilioAuthTokenEnv = std::getenv("TWILIO_AUTH_TOKEN");
    std::string twilioAuthToken = twilioAuthTokenEnv ? twilioAuthTokenEnv : "";
    if (!twilioAuthToken.empty()) {
        std::cout << "[Server] Twilio Auth Token detected. Signature validation is ACTIVE." << std::endl;
    } else {
        std::cout << "[Server] TWILIO_AUTH_TOKEN not set. Signature validation is bypassed (development mode)." << std::endl;
    }

    httplib::Server svr;

    // Static files handler (mounts frontend assets)
    svr.set_mount_point("/", publicDir);

    // /suggest?mode=autocomplete|spellcheck|paragraph&q=...
    svr.Get("/suggest", [&](const httplib::Request& req, httplib::Response& res) {
        std::string mode = req.has_param("mode") ? req.get_param_value("mode") : "autocomplete";
        std::string q = req.has_param("q") ? req.get_param_value("q") : "";

        std::ostringstream json;

        if (mode == "autocomplete") {
            auto suggestions = autocomplete.suggest(q);
            json << "{\n";
            json << "  \"mode\": \"autocomplete\",\n";
            json << "  \"query\": \"" << escapeJson(q) << "\",\n";
            json << "  \"count\": " << suggestions.size() << ",\n";
            json << "  \"suggestions\": [";
            for (size_t i = 0; i < suggestions.size(); ++i) {
                if (i > 0) json << ", ";
                json << "\"" << escapeJson(suggestions[i]) << "\"";
            }
            json << "]\n}";
        } else if (mode == "spellcheck") {
            bool correct = spellChecker.isCorrect(q);
            auto candidates = spellChecker.suggestWithDistance(q);

            json << "{\n";
            json << "  \"mode\": \"spellcheck\",\n";
            json << "  \"query\": \"" << escapeJson(q) << "\",\n";
            json << "  \"isCorrect\": " << (correct ? "true" : "false") << ",\n";
            json << "  \"count\": " << candidates.size() << ",\n";
            json << "  \"suggestions\": [";
            for (size_t i = 0; i < candidates.size(); ++i) {
                if (i > 0) json << ",\n    ";
                else json << "\n    ";
                json << "{\"word\": \"" << escapeJson(candidates[i].first)
                     << "\", \"distance\": " << candidates[i].second << "}";
            }
            if (!candidates.empty()) json << "\n  ";
            json << "]\n}";
        } else if (mode == "paragraph") {
            if (q.size() > 5000) {
                res.status = 413;
                res.set_content("{\"error\": \"Paragraph too long (max 5000 characters)\"}", "application/json");
                return;
            }
            ParagraphChecker::Report report = paragraphChecker.analyze(q);

            json << "{\n";
            json << "  \"mode\": \"paragraph\",\n";
            json << "  \"totalWords\": " << report.totalWords << ",\n";
            json << "  \"checkedWords\": " << report.checkedWords << ",\n";
            json << "  \"misspelledCount\": " << report.misspelledWords << ",\n";
            json << "  \"truncated\": " << (report.truncated ? "true" : "false") << ",\n";
            json << "  \"words\": [";
            for (size_t i = 0; i < report.words.size(); ++i) {
                const auto& w = report.words[i];
                const char* status = w.status == ParagraphChecker::Status::Correct ? "correct"
                                   : w.status == ParagraphChecker::Status::Misspelled ? "misspelled" : "skipped";
                if (i > 0) json << ",";
                json << "\n    {\"word\": \"" << escapeJson(w.word) << "\", \"status\": \"" << status << "\"";
                if (w.status == ParagraphChecker::Status::Misspelled) {
                    json << ", \"suggestions\": [";
                    for (size_t k = 0; k < w.suggestions.size(); ++k) {
                        if (k > 0) json << ", ";
                        json << "{\"word\": \"" << escapeJson(w.suggestions[k].first)
                             << "\", \"distance\": " << w.suggestions[k].second << "}";
                    }
                    json << "]";
                }
                json << "}";
            }
            if (!report.words.empty()) json << "\n  ";
            json << "]\n}";
        } else if (mode == "grammar") {
            if (q.size() > 5000) {
                res.status = 413;
                res.set_content("{\"error\": \"Text too long (max 5000 characters)\"}", "application/json");
                return;
            }
            GrammarChecker::Report report = grammarChecker.analyze(q);

            json << "{\n";
            json << "  \"mode\": \"grammar\",\n";
            json << "  \"totalWords\": " << report.totalWords << ",\n";
            json << "  \"sentenceCount\": " << report.sentenceCount << ",\n";
            json << "  \"issueCount\": " << report.issues.size() << ",\n";
            json << "  \"corrected\": \"" << escapeJson(report.corrected) << "\",\n";
            json << "  \"issues\": [";
            for (size_t i = 0; i < report.issues.size(); ++i) {
                const auto& issue = report.issues[i];
                const char* sev = issue.severity == GrammarChecker::Severity::Error ? "error"
                                : issue.severity == GrammarChecker::Severity::Warning ? "warning" : "style";
                if (i > 0) json << ",";
                json << "\n    {"
                     << "\"position\": " << issue.position << ", "
                     << "\"length\": " << issue.length << ", "
                     << "\"original\": \"" << escapeJson(issue.original) << "\", "
                     << "\"suggestion\": \"" << escapeJson(issue.suggestion) << "\", "
                     << "\"rule\": \"" << escapeJson(issue.rule) << "\", "
                     << "\"message\": \"" << escapeJson(issue.message) << "\", "
                     << "\"severity\": \"" << sev << "\""
                     << "}";
            }
            if (!report.issues.empty()) json << "\n  ";
            json << "]\n}";
        } else {
            res.status = 400;
            res.set_content("{\"error\": \"Invalid mode. Use 'autocomplete', 'spellcheck', 'paragraph' or 'grammar'\"}", "application/json");
            return;
        }

        res.set_content(json.str(), "application/json");
    });

    // GET /game/new?level=1|2|3 -> {id, scrambled, level}
    svr.Get("/game/new", [&](const httplib::Request& req, httplib::Response& res) {
        int level = 1;
        if (req.has_param("level")) {
            try {
                level = std::stoi(req.get_param_value("level"));
            } catch (...) {
                level = 1;
            }
        }
        auto session = spellingGame.startNewGame(level);

        std::ostringstream json;
        json << "{\n"
             << "  \"id\": \"" << escapeJson(session.id) << "\",\n"
             << "  \"scrambled\": \"" << escapeJson(session.scrambledWord) << "\",\n"
             << "  \"level\": " << session.level << "\n"
             << "}";
        res.set_content(json.str(), "application/json");
    });

    // GET /game/answer?id=...&a=... -> {correct, answer, distance, points, totalScore, streak, message}
    svr.Get("/game/answer", [&](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.has_param("id") ? req.get_param_value("id") : "";
        std::string ans = req.has_param("a") ? req.get_param_value("a") : "";

        if (id.empty()) {
            res.status = 400;
            res.set_content("{\"error\": \"Missing game session id\"}", "application/json");
            return;
        }

        auto result = spellingGame.submitAnswer(id, ans);

        std::ostringstream json;
        json << "{\n"
             << "  \"correct\": " << (result.correct ? "true" : "false") << ",\n"
             << "  \"answer\": \"" << escapeJson(result.answer) << "\",\n"
             << "  \"targetWord\": \"" << escapeJson(result.targetWord) << "\",\n"
             << "  \"distance\": " << result.distance << ",\n"
             << "  \"points\": " << result.points << ",\n"
             << "  \"totalScore\": " << result.totalScore << ",\n"
             << "  \"streak\": " << result.streak << ",\n"
             << "  \"message\": \"" << escapeJson(result.message) << "\"\n"
             << "}";
        res.set_content(json.str(), "application/json");
    });

    // POST /whatsapp -> Twilio WhatsApp webhook responding with TwiML
    svr.Post("/whatsapp", [&](const httplib::Request& req, httplib::Response& res) {
        std::string sender = req.has_param("From") ? req.get_param_value("From") : "unknown";
        std::string body = req.has_param("Body") ? req.get_param_value("Body") : "";

        // 1. Rate limiting check (20 msgs / min per sender)
        if (!whatsAppBot.checkRateLimit(sender)) {
            res.status = 429;
            res.set_content(WhatsAppBot::toTwiML("⚠️ Rate limit exceeded. Please wait a minute before sending more messages."), "text/xml; charset=utf-8");
            return;
        }

        // 2. Validate X-Twilio-Signature if TWILIO_AUTH_TOKEN is configured
        if (!twilioAuthToken.empty()) {
            std::string sig = req.get_header_value("X-Twilio-Signature");
            // Reconstruct full request URL: check X-Forwarded-Proto / Host or Host header
            std::string host = req.get_header_value("Host");
            if (host.empty()) host = "localhost:" + std::to_string(port);
            std::string proto = req.get_header_value("X-Forwarded-Proto");
            if (proto.empty()) proto = "http";
            std::string fullUrl = proto + "://" + host + req.path;

            if (!WhatsAppBot::validateTwilioSignature(twilioAuthToken, fullUrl, req.params, sig)) {
                std::cerr << "[Security] Twilio signature validation failed for sender: " << sender << std::endl;
                res.status = 403;
                res.set_content("Forbidden: Invalid Twilio Signature", "text/plain");
                return;
            }
        }

        // 3. Process message and reply with TwiML
        std::string botReply = whatsAppBot.reply(sender, body);
        std::string twiml = WhatsAppBot::toTwiML(botReply);

        res.status = 200;
        res.set_content(twiml, "text/xml; charset=utf-8");
    });

    std::cout << "[Server] Server listening on port " << port << std::endl;
    std::cout << "[Server] Open http://localhost:" << port << " in your web browser." << std::endl;
    svr.listen("0.0.0.0", port);
}
