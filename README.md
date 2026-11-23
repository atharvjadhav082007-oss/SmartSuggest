# Autocomplete, Spell Checker & WhatsApp Suite in C++ (C++17)

A full-stack, high-performance **Autocomplete, Spell Checker, Spelling Challenge Game, and WhatsApp Webhook Bot** written in **C++17** utilizing core **Object-Oriented Programming (OOP)** paradigms. It features a prefix **Trie** with Depth-First Search (DFS), pure recursive **Levenshtein edit distance**, an interactive console CLI, an embedded HTTP web server with REST & Twilio WhatsApp endpoints, and a modern glassmorphism web frontend.

---

## 🏛️ OOP Concepts Demonstrated

| OOP Concept | Implementation Details |
| :--- | :--- |
| **Encapsulation** | • **`Dictionary`**: Internal word list is private; words are loaded, sanitized, and exposed strictly via `getWords()` const reference.<br>• **`Trie`**: Internal root node pointer, 26-ary branching, and recursive DFS are fully encapsulated.<br>• **`SpellingGame`**: Internal session map, active word targets, streaks, scores, and expiration timestamps are private and synchronized.<br>• **`WhatsAppBot`**: Per-user conversation state and rate-limiting counters are encapsulated behind clean method interfaces.<br>• **`CryptoUtils`**: Bitwise SHA-1 compression rounds and HMAC padding logic are encapsulated. |
| **Abstraction** | • **`Suggester`**: Abstract base class defining the contract `virtual std::vector<std::string> suggest(const std::string& query) = 0;`.<br>• Consumers interact cleanly without needing to know whether prefix matching, edit distance, paragraph tokenization, or challenge candidate retrieval is happening. |
| **Inheritance** | • **`Autocomplete`** publicly inherits from `Suggester`.<br>• **`SpellChecker`** publicly inherits from `Suggester`.<br>• **`ParagraphChecker`** publicly inherits from `Suggester`.<br>• **`SpellingGame`** publicly inherits from `Suggester`. |
| **Polymorphism** | • Runtime dynamic dispatch via `Suggester*` base pointers: calling `suggester->suggest(...)` dynamically dispatches to `Autocomplete`, `SpellChecker`, `ParagraphChecker`, or `SpellingGame`. |
| **Composition & Delegation** | • **`ParagraphChecker`** inherits from `Suggester` and HAS-A `SpellChecker`: tokenizes paragraphs and delegates word analysis to the spell checker.<br>• **`WhatsAppBot`** HAS-A `Autocomplete`, `SpellChecker`, `ParagraphChecker`, and `SpellingGame`: delegates all commands to their specialized engines without duplicating NLP or game logic. |
| **Constructors & Destructors (RAII)** | • **`TrieNode`**: Destructor recursively deletes all non-null child nodes.<br>• **`LockGuard` & `Mutex`**: Strict RAII scoped locking primitive protecting concurrent game sessions and rate limits.<br>• **Copy Prevention**: `Trie`, `TrieNode`, and `Mutex` delete copy constructors (`= delete`) preventing shallow copies and resource leaks. |

---

## 📁 Project Structure

```
oops cp/
├── include/
│   ├── Dictionary.h       # Encapsulated dictionary loader and normalizer
│   ├── TrieNode.h         # 26-ary node with recursive RAII destructor
│   ├── Trie.h             # Prefix tree with DFS word retrieval
│   ├── Suggester.h        # Abstract Base Class (pure virtual suggest)
│   ├── Autocomplete.h     # Derived suggester (prefix search up to 10 words)
│   ├── SpellChecker.h     # Derived suggester (Levenshtein distance <= 2, up to 5 words)
│   ├── ParagraphChecker.h # Derived suggester (tokenizes paragraph, checks words)
│   ├── SpellingGame.h     # Derived suggester (challenge game engine & scoring)
│   ├── WhatsAppBot.h      # Twilio WhatsApp chatbot conversation controller
│   ├── CryptoUtils.h      # Self-contained SHA-1, HMAC-SHA1 & Base64 encoder
│   ├── MutexRAII.h        # Lightweight cross-platform RAII mutex & lock guard
│   └── httplib.h          # Custom HTTP server with GET/POST, URL & form decoding
├── src/
│   ├── Dictionary.cpp
│   ├── TrieNode.cpp
│   ├── Trie.cpp
│   ├── Levenshtein.cpp
│   ├── Autocomplete.cpp
│   ├── SpellChecker.cpp
│   ├── ParagraphChecker.cpp
│   ├── SpellingGame.cpp
│   ├── GrammarChecker.cpp
│   ├── WhatsAppBot.cpp
│   ├── server.cpp         # HTTP REST endpoints & WhatsApp webhook handler
│   ├── main.cpp           # Interactive console CLI menu (options 1-7)
│   └── test_cases.cpp     # Automated test suite (13 unit & OOP tests)
├── frontend/              # Modern React 18 + Vite frontend source
│   ├── src/
│   │   ├── components/    # Header, NavTabs, AutocompleteView, SpellCheckView,
│   │   │                  # ParagraphView, GrammarView, GameView, OopInspector, etc.
│   │   ├── App.jsx        # Top-level state & view controller
│   │   └── main.jsx
│   ├── package.json
│   └── vite.config.js     # Builds production assets directly into public/
├── public/                # Built production assets served by C++ server
├── public_vanilla/        # Original vanilla HTML/JS backup
├── words.txt              # Dictionary wordlist (~380 words)
├── build.bat              # Windows compilation batch script
├── Makefile               # GNU Make configuration
└── README.md
```

---

## ⚡ Compilation Instructions

### Option 1: Using Windows Build Script
```cmd
build.bat
```
Produces:
- `app.exe`: Full interactive application (Console CLI + embedded Web Server).
- `test_runner.exe`: Automated unit test runner.

### Option 2: Using GNU Make
```bash
make
```

### Option 3: Direct g++ Invocation
```bash
# Main app + server
g++ -std=c++17 -Wall -Wextra -Iinclude src/Dictionary.cpp src/TrieNode.cpp src/Trie.cpp src/Levenshtein.cpp src/Autocomplete.cpp src/SpellChecker.cpp src/ParagraphChecker.cpp src/SpellingGame.cpp src/WhatsAppBot.cpp src/server.cpp src/main.cpp -o app.exe -lws2_32

# Test suite
g++ -std=c++17 -Wall -Wextra -Iinclude src/Dictionary.cpp src/TrieNode.cpp src/Trie.cpp src/Levenshtein.cpp src/Autocomplete.cpp src/SpellChecker.cpp src/ParagraphChecker.cpp src/SpellingGame.cpp src/WhatsAppBot.cpp src/test_cases.cpp -o test_runner.exe -lws2_32
```
---

## 🚀 Running the Project

### 1. Interactive Console Menu
```bash
.\app.exe
```
Menu options:
1. **Autocomplete**: Prefix matching with DFS traversal.
2. **Spell Check**: Levenshtein edit distance $\le 2$.
3. **Paragraph Spell Check**: Full paragraph analysis and auto-correct.
4. **Spelling Challenge Game**: Interactive unscramble/spelling challenge with scoring.
5. **Start Web Server**: Launches HTTP server on port 8080.
6. **Exit**.

### 2. Standalone Web Server
```bash
.\app.exe --server
# Or specify a custom port:
.\app.exe --server 9000
```
Open **`http://localhost:8080`** in any web browser.

### 3. Running Unit Tests
```bash
.\test_runner.exe
```
Executes all **13 automated tests** covering Levenshtein distance, Dictionary encapsulation, Trie DFS, Suggesters, ParagraphChecker, Form-urlencoded decoding, XML escaping, SpellingGame scoring & corruption, WhatsAppBot commands, rate limiting, and HMAC-SHA1 signature verification.

---

## 🌐 Web & API Endpoints

| Method | Endpoint | Query / Body Params | Response |
| :--- | :--- | :--- | :--- |
| `GET` | `/suggest` | `mode=autocomplete\|spellcheck\|paragraph&q=...` | JSON suggestions or paragraph report |
| `GET` | `/game/new` | `level=1\|2\|3` | `{ "id": "...", "scrambled": "...", "level": 1 }` |
| `GET` | `/game/answer` | `id=...&a=...` | `{ "correct": bool, "distance": int, "points": int, "totalScore": int, "streak": int }` |
| `POST` | `/whatsapp` | `From=+...&Body=...` (form-urlencoded) | TwiML XML (`<Response><Message>...</Message></Response>`) |

---

## 📱 WhatsApp Integration Setup

### A) Share on WhatsApp (Frontend)
- Located on the **Paragraph** tab in the web dashboard.
- Clicking **Share** allows sending:
  1. **Auto-Corrected Text**: Sends clean, typo-fixed text directly via `https://wa.me/?text=...`.
  2. **Original Text + Mistakes Report**: Sends original input and bulleted list of detected typos and suggestions.
- Automatically cuts text to safe length (< 1500 chars) and uses `encodeURIComponent`.

### B) Twilio WhatsApp Webhook Bot (Backend)
1. **Create a Twilio Account & Sandbox**:
   - Go to [Twilio Console](https://console.twilio.com/) → Messaging → Try WhatsApp.
   - Join your sandbox by sending the join phrase (e.g., `join <sandbox-word>`) from your WhatsApp to the Twilio number.
2. **Expose Local Server via ngrok**:
   ```bash
   ngrok http 8080
   ```
3. **Configure Twilio Webhook**:
   - In Twilio Sandbox settings, set **"WHEN A MESSAGE COMES IN"** to:
     `https://<your-ngrok-subdomain>.ngrok-free.app/whatsapp`
   - Set HTTP Method to **HTTP POST**.
4. **Security & Signature Verification (Optional but Recommended)**:
   - Set your Twilio Auth Token in your environment:
     ```cmd
     set TWILIO_AUTH_TOKEN=your_auth_token_here
     .\app.exe --server
     ```
   - When set, incoming requests will validate the `X-Twilio-Signature` using pure C++ HMAC-SHA1 + Base64.
5. **Chatbot Commands**:
   - `help` : View command list.
   - `check <text>` : Report typos and suggestions.
   - `fix <text>` : Return auto-corrected paragraph.
   - `complete <prefix>` : Up to 10 autocomplete suggestions.
   - `game` : Start the spelling challenge game. The next reply is evaluated as your answer.
   - Any regular text is checked automatically.

---

## ☁️ Deploying the Backend on Render

The repo includes a `Dockerfile` and a `render.yaml` blueprint that build the C++ server on Linux and run it with `./app --server`. The server reads the port from the `PORT` environment variable that Render provides.

1. In Render, choose **New → Blueprint** and select this repository. Render creates the `smartsuggest-api` web service from `render.yaml`.
2. Optionally set `TWILIO_AUTH_TOKEN` on the service to turn on WhatsApp signature checks.
3. Copy the service URL (for example `https://smartsuggest-api.onrender.com`).
4. On Netlify, set the environment variable `VITE_API_BASE_URL` to that URL and redeploy the site so the frontend calls the Render backend.
5. For the WhatsApp bot, set the Twilio webhook to `https://<your-render-url>/whatsapp` (HTTP POST).
