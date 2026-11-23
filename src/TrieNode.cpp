#include "TrieNode.h"
#include <cctype>

TrieNode::TrieNode() : isEndOfWord(false) {
    children.fill(nullptr);
}

TrieNode::~TrieNode() {
    for (size_t i = 0; i < ALPHABET_SIZE; ++i) {
        if (children[i] != nullptr) {
            delete children[i];
            children[i] = nullptr;
        }
    }
}

TrieNode::TrieNode(TrieNode&& other) noexcept : isEndOfWord(other.isEndOfWord) {
    children = other.children;
    other.children.fill(nullptr);
    other.isEndOfWord = false;
}

TrieNode& TrieNode::operator=(TrieNode&& other) noexcept {
    if (this != &other) {
        // Free existing child subtree
        for (size_t i = 0; i < ALPHABET_SIZE; ++i) {
            delete children[i];
        }
        children = other.children;
        isEndOfWord = other.isEndOfWord;

        other.children.fill(nullptr);
        other.isEndOfWord = false;
    }
    return *this;
}

int TrieNode::charToIndex(char c) {
    unsigned char lower = static_cast<unsigned char>(std::tolower(c));
    if (lower >= 'a' && lower <= 'z') {
        return lower - 'a';
    }
    return -1;
}

char TrieNode::indexToChar(int index) {
    return static_cast<char>('a' + index);
}

