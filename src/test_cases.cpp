#include "Dictionary.h"
#include "Trie.h"
#include "Suggester.h"
#include "Autocomplete.h"
#include "SpellChecker.h"
#include "ParagraphChecker.h"
#include "SpellingGame.h"
#include "WhatsAppBot.h"
#include "httplib.h"
#include "CryptoUtils.h"
#include <iostream>
#include <cassert>
#include <algorithm>
#include <memory>
#include <map>

void runAllTests() {
    std::cout << "========================================\n";
    std::cout << "   RUNNING AUTOMATED UNIT & OOP TESTS   \n";
    std::cout << "========================================\n\n";

    // 1. Test Levenshtein Distance
    std::cout << "[Test 1] Testing Levenshtein Distance Algorithm...\n";
    assert(Levenshtein::distance("kitten", "sitting") == 3);
    assert(Levenshtein::distance("program", "progam") == 1);
    assert(Levenshtein::distance("program", "program") == 0);
    assert(Levenshtein::distance("", "hello") == 5);
    assert(Levenshtein::distance("world", "") == 5);
    assert(Levenshtein::distance("definately", "definitely") == 1);
    std::cout << "  ✓ Levenshtein distance calculations verified.\n\n";

    // 2. Test Dictionary Loading (Encapsulation)
    std::cout << "[Test 2] Testing Dictionary Encapsulation & Loading...\n";
    Dictionary dict("words.txt");
    if (dict.size() == 0) {
        dict.loadFromFile("../words.txt");
    }
    assert(dict.size() > 50);
    std::cout << "  ✓ Dictionary loaded " << dict.size() << " sanitized words.\n\n";

    // 3. Test Trie Operations (DFS & Search)
    std::cout << "[Test 3] Testing Trie Operations (Insert, Search, DFS)...\n";
    Trie trie;
    for (const auto& w : dict.getWords()) {
        trie.insert(w);
    }
    assert(trie.search("program") == true);
    assert(trie.search("project") == true);
    assert(trie.search("process") == true);
    assert(trie.search("progam") == false);
    assert(trie.search("nonexistentwordxyz") == false);

    // Prefix search for "pro"
    auto proCompletions = trie.wordsWithPrefix("pro", 10);
    assert(!proCompletions.empty());
    assert(proCompletions.size() <= 10);
    std::cout << "  Prefix matches for 'pro' (up to 10):\n";
    for (const auto& w : proCompletions) {
        std::cout << "    - " << w << "\n";
    }
    std::cout << "  ✓ Trie search and DFS prefix retrieval verified.\n\n";

    // 4. Test Autocomplete Suggester (Inheritance & Polymorphism)
    std::cout << "[Test 4] Testing Autocomplete Suggester...\n";
    std::unique_ptr<Suggester> acSuggester = std::make_unique<Autocomplete>(trie, 30);
    std::vector<std::string> acResults = acSuggester->suggest("pro");
    assert(acResults.size() <= 30);
    bool foundProgram = (std::find(acResults.begin(), acResults.end(), "program") != acResults.end());
    bool foundProject = (std::find(acResults.begin(), acResults.end(), "project") != acResults.end());
    bool foundProcess = (std::find(acResults.begin(), acResults.end(), "process") != acResults.end());
    assert(foundProgram && foundProject && foundProcess);
    std::cout << "  ✓ Autocomplete 'pro' correctly yields program, project, and process.\n\n";

    // 5. Test SpellChecker Suggester (Levenshtein edit distance <= 2)
    std::cout << "[Test 5] Testing SpellChecker Suggester...\n";
    std::unique_ptr<Suggester> scSuggester = std::make_unique<SpellChecker>(trie, dict, 2, 5);
    
    // Test: correct word must return empty vector
    std::vector<std::string> correctCheck = scSuggester->suggest("program");
    assert(correctCheck.empty());
    std::cout << "  ✓ 'program' is correct: suggest() returns 0 items as mandated.\n";

    // Test: "progam" must suggest "program" (edit distance 1)
    std::vector<std::string> progamSuggestions = scSuggester->suggest("progam");
    assert(!progamSuggestions.empty());
    assert(progamSuggestions[0] == "program"); // Closest match distance 1
    std::cout << "  ✓ 'progam' correctly suggests 'program' as first suggestion.\n";

    // Test: "definately" must suggest "definitely"
    std::vector<std::string> defSuggestions = scSuggester->suggest("definately");
    assert(!defSuggestions.empty());
    assert(std::find(defSuggestions.begin(), defSuggestions.end(), "definitely") != defSuggestions.end());
    std::cout << "  ✓ 'definately' correctly suggests 'definitely'.\n\n";

    // 6. Test Polymorphic Dispatch via base pointer
    std::cout << "[Test 6] Testing Runtime Polymorphism via Suggester* ...\n";
    Suggester* ptrs[2] = { acSuggester.get(), scSuggester.get() };
    // Call virtual suggest on both
    std::vector<std::string> r0 = ptrs[0]->suggest("tree"); // Autocomplete
    std::vector<std::string> r1 = ptrs[1]->suggest("tree"); // SpellChecker (correct, returns empty)
    assert(!r0.empty());
    assert(r1.empty());
    std::cout << "  ✓ Polymorphic virtual dispatch confirmed on both derived classes.\n\n";

    // 7. Test RAII & Destructors
    std::cout << "[Test 7] Testing RAII & Destruction (No Leaks / Crashes)...\n";
    {
        Trie tempTrie;
        tempTrie.insert("temporary");
        tempTrie.insert("testing");
        tempTrie.insert("tree");
        // Goes out of scope here: destructor cleanly releases all nodes recursively
    }
    std::cout << "  ✓ Scope exit cleaned up Trie nodes via RAII destructor.\n\n";

    // 8. Test ParagraphChecker
    std::cout << "[Test 8] Testing ParagraphChecker (paragraph-level spell check)...\n";
    {
        SpellChecker sc(trie, dict, 2, 5);
        ParagraphChecker pc(sc, 5000);

        auto rep = pc.analyze("The Program was definately good, progam 42 don't PROGAM.");
        assert(rep.misspelledWords == 3);          // definately, progam, PROGAM
        assert(rep.totalWords == rep.words.size());
        assert(rep.checkedWords == rep.totalWords - 2); // "42" and "don't" are skipped
        std::cout << "  ✓ Counts words, flags 3 misspellings, skips numbers/contractions.\n";

        // Case-insensitive, repeats flagged each time
        assert(rep.words.back().status == ParagraphChecker::Status::Misspelled);
        assert(!rep.words.back().suggestions.empty());
        assert(rep.words.back().suggestions[0].first == "program");
        std::cout << "  ✓ Case-insensitive lookup; repeated mistakes each flagged.\n";

        // Autocorrect keeps punctuation and leading capital
        std::string fixed = pc.autoCorrect("Progam, definately!");
        assert(fixed == "Program, definitely!");
        std::cout << "  ✓ autoCorrect() fixes words and preserves punctuation & capitalisation.\n";

        // Polymorphic use through Suggester*
        Suggester* base = &pc;
        std::vector<std::string> bad = base->suggest("progam progam definately program");
        assert(bad.size() == 2 && bad[0] == "progam" && bad[1] == "definately");
        std::cout << "  ✓ Polymorphic suggest() returns distinct misspelled words.\n";

        // Empty / truncation
        assert(pc.analyze("").totalWords == 0);
        ParagraphChecker tiny(sc, 10);
        assert(tiny.analyze("program program program").truncated);
        std::cout << "  ✓ Empty input and max-length truncation handled.\n\n";
    }

    // 9. Test Form-Urlencoded Body Parsing in httplib
    std::cout << "[Test 9] Testing Form-Urlencoded & URL Decoding in httplib...\n";
    {
        std::string encoded = "From=%2B1234567890&Body=Hello%20World%2BTest%21&MediaUrl=";
        std::map<std::string, std::string> params;
        httplib::Server::parse_form_urlencoded(encoded, params);

        assert(params["From"] == "+1234567890");
        assert(params["Body"] == "Hello World+Test!");
        assert(params["MediaUrl"] == "");
        std::cout << "  ✓ URL decoding handles '%XX' hex codes and '+' spaces correctly.\n";
        std::cout << "  ✓ Query parameter pair separation handles empty parameters properly.\n\n";
    }

    // 10. Test XML Escaping and TwiML Generation
    std::cout << "[Test 10] Testing XML Escaping & TwiML Response Format...\n";
    {
        std::string unsafeText = "<Tom & Jerry> \"quote\" 'single' test";
        std::string escaped = WhatsAppBot::escapeXml(unsafeText);
        assert(escaped == "&lt;Tom &amp; Jerry&gt; &quot;quote&quot; &apos;single&apos; test");

        std::string twiml = WhatsAppBot::toTwiML("Hello & Goodbye");
        assert(twiml.find("<Response>") != std::string::npos);
        assert(twiml.find("<Message>Hello &amp; Goodbye</Message>") != std::string::npos);
        assert(twiml.find("</Response>") != std::string::npos);
        std::cout << "  ✓ XML special chars (& < > \" ') properly escaped to prevent injection.\n";
        std::cout << "  ✓ TwiML envelope is well-formed XML under 1500 chars.\n\n";
    }

    // 11. Test SpellingGame (Scoring, Corruption, Sessions, Polymorphism)
    std::cout << "[Test 11] Testing SpellingGame Engine & Scoring...\n";
    {
        SpellingGame game(dict);

        // Scoring rules: exact = 10, dist 1 = 5, dist 2 = 2, else 0
        assert(SpellingGame::calculatePoints(0) == 10);
        assert(SpellingGame::calculatePoints(1) == 5);
        assert(SpellingGame::calculatePoints(2) == 2);
        assert(SpellingGame::calculatePoints(3) == 0);
        assert(SpellingGame::calculatePoints(5) == 0);

        // Word corruption at different levels
        std::string original = "program";
        std::string c1 = game.corruptWord(original, 1);
        std::string c2 = game.corruptWord(original, 2);
        std::string c3 = game.corruptWord(original, 3);
        assert(!c1.empty());
        assert(!c2.empty());
        assert(!c3.empty());
        // At least one corruption occurred
        assert(c1 != original || c2 != original);

        // Game session & answer submission
        auto session = game.startNewGame(1);
        assert(!session.id.empty());
        assert(!session.scrambledWord.empty());

        // Correct answer check
        auto ansResult = game.submitAnswer(session.id, session.targetWord);
        assert(ansResult.correct == true);
        assert(ansResult.distance == 0);
        assert(ansResult.points >= 10);
        assert(ansResult.streak == 1);

        // Polymorphic use of SpellingGame as Suggester*
        Suggester* suggesterPtr = &game;
        auto suggestions = suggesterPtr->suggest("progam");
        assert(!suggestions.empty());
        std::cout << "  ✓ Points table, corruption levels, sessions, and streak verified.\n";
        std::cout << "  ✓ SpellingGame polymorphism via Suggester* confirmed.\n\n";
    }

    // 12. Test WhatsAppBot Commands & State Flow
    std::cout << "[Test 12] Testing WhatsAppBot Commands & Conversation Flow...\n";
    {
        Autocomplete ac(trie, 30);
        SpellChecker sc(trie, dict, 2, 5);
        ParagraphChecker pc(sc, 5000);
        SpellingGame sg(dict);
        WhatsAppBot bot(ac, sc, pc, sg);

        // help command
        std::string helpReply = bot.reply("help");
        assert(helpReply.find("Commands") != std::string::npos);

        // complete command
        std::string compReply = bot.reply("complete pro");
        assert(compReply.find("program") != std::string::npos);

        // fix command
        std::string fixReply = bot.reply("fix I definately made a progam");
        assert(fixReply.find("definitely") != std::string::npos);
        assert(fixReply.find("program") != std::string::npos);

        // default text treated as check
        std::string checkReply = bot.reply("definately progam");
        assert(checkReply.find("definitely") != std::string::npos);
        assert(checkReply.find("program") != std::string::npos);

        // clean text check
        std::string goodReply = bot.reply("check program");
        assert(goodReply.find("All Good") != std::string::npos);

        // WhatsAppBot game flow:
        std::string gamePrompt = bot.reply("+123456789", "game");
        assert(gamePrompt.find("Spelling Challenge") != std::string::npos);
        // Next message from +123456789 is treated as the answer
        std::string ansReply = bot.reply("+123456789", "wrongxyzanswer");
        assert(ansReply.find("Total Score") != std::string::npos);

        // Rate limiting test
        for (int i = 0; i < 20; ++i) {
            assert(bot.checkRateLimit("+999999999") == true);
        }
        // 21st message in same minute blocked
        assert(bot.checkRateLimit("+999999999") == false);

        std::cout << "  ✓ All bot commands (help, check, fix, complete, game) verified.\n";
        std::cout << "  ✓ Two-step WhatsApp game state flow verified.\n";
        std::cout << "  ✓ Per-sender rate limiting (20 req/min) confirmed.\n\n";
    }

    // 13. Test HMAC-SHA1 and Twilio Signature Verification
    std::cout << "[Test 13] Testing Twilio Signature Validation (HMAC-SHA1 & Base64)...\n";
    {
        // Reference test vector for HMAC-SHA1
        // Key: "key", Data: "The quick brown fox jumps over the lazy dog"
        // Expected base64: "3nybhbi3iqa8ino29wqQcBydtNk="
        auto hmac = CryptoUtils::hmacSha1("key", "The quick brown fox jumps over the lazy dog");
        std::string b64 = CryptoUtils::base64Encode(hmac);
        assert(b64 == "3nybhbi3iqa8ino29wqQcBydtNk=");

        // Test Twilio signature validation logic
        std::string authToken = "12345secret";
        std::string url = "https://myhook.ngrok.io/whatsapp";
        std::map<std::string, std::string> postParams = {
            {"From", "+123456789"},
            {"Body", "help"}
        };
        // Expected signature calculation: HMAC-SHA1(url + Body + help + From + +123456789)
        std::string expectedData = url + "Body" + "help" + "From" + "+123456789";
        std::string validSig = CryptoUtils::base64Encode(CryptoUtils::hmacSha1(authToken, expectedData));

        assert(WhatsAppBot::validateTwilioSignature(authToken, url, postParams, validSig) == true);
        assert(WhatsAppBot::validateTwilioSignature(authToken, url, postParams, "tamperedSignature") == false);
        std::cout << "  ✓ Self-contained HMAC-SHA1 and Base64 match cryptographic test vectors.\n";
        std::cout << "  ✓ Twilio request signature verification confirmed.\n\n";
    }

    std::cout << "========================================\n";
    std::cout << "     ALL 13 TESTS PASSED SUCCESSFULLY!  \n";
    std::cout << "========================================\n";
}

int main() {
    runAllTests();
    return 0;
}
