#include "Trie.h"
#include <cctype>

Trie::Trie() : root(new TrieNode()) {}

Trie::~Trie() {
    delete root;
    root = nullptr;
}



Trie::Trie(Trie&& other) noexcept : root(other.root) {
    other.root = nullptr;
}


Trie& Trie::operator=(Trie&& other) noexcept {
    if (this != &other) {
        delete root;
        root = other.root;
        other.root = nullptr;
    }
    return *this;
}


void Trie::insert(const std::string& word) {
    if (!root) {
        root = new TrieNode();
    }

    TrieNode* curr = root;
    for (char c : word) {
        int idx = TrieNode::charToIndex(c);
        if (idx == -1) {
            continue; // Ignore non-alphabet characters
        }
        if (curr->children[idx] == nullptr) {
            curr->children[idx] = new TrieNode();
        }
        curr = curr->children[idx];
    }
    curr->isEndOfWord = true;
}

bool Trie::search(const std::string& word) const {
    if (!root || word.empty()) {
        return false;
    }

    TrieNode* curr = root;
    for (char c : word) {
        int idx = TrieNode::charToIndex(c);
        if (idx == -1) {
            return false;
        }
        if (curr->children[idx] == nullptr) {
            return false;
        }
        curr = curr->children[idx];
    }
    return curr->isEndOfWord;
}

void Trie::dfs(TrieNode* node, std::string& currentWord, std::vector<std::string>& results, size_t maxCount) const {
    if (!node || results.size() >= maxCount) {
        return;
    }

    if (node->isEndOfWord) {
        results.push_back(currentWord);
        if (results.size() >= maxCount) {
            return;
        }
    }

    for (size_t i = 0; i < TrieNode::ALPHABET_SIZE; ++i) {
        if (node->children[i] != nullptr) {
            currentWord.push_back(TrieNode::indexToChar(static_cast<int>(i)));
            dfs(node->children[i], currentWord, results, maxCount);
            currentWord.pop_back();

            if (results.size() >= maxCount) {
                return;
            }
        }
    }
}

std::vector<std::string> Trie::wordsWithPrefix(const std::string& prefix, size_t maxCount) const {
    std::vector<std::string> results;
    if (!root || prefix.empty()) {
        return results;
    }

    TrieNode* curr = root;
    std::string normalizedPrefix;
    normalizedPrefix.reserve(prefix.size());

    for (char c : prefix) {
        int idx = TrieNode::charToIndex(c);
        if (idx == -1) {
            return results; // Invalid character in prefix
        }
        normalizedPrefix.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        if (curr->children[idx] == nullptr) {
            return results; // Prefix not found in Trie
        }
        curr = curr->children[idx];
    }

    // Perform DFS traversal starting from the prefix node
    std::string currentWord = normalizedPrefix;
    dfs(curr, currentWord, results, maxCount);

    return results;
}

void Trie::clear() {
    delete root;
    root = new TrieNode();
}

