#ifndef TRIENODE_H
#define TRIENODE_H

#include <array>
#include <cstddef>

/**
 * @class TrieNode
 * @brief Represents a single character node in a 26-ary prefix Trie.
 *
 * Demonstrates:
 * - Encapsulation (state and pointers managed internally)
 * - Constructors and Destructors (recursive cleanup of child nodes via RAII)
 * - Copy Prevention (copy constructor and copy assignment are deleted)
 */
class TrieNode {
public:
    static constexpr size_t ALPHABET_SIZE = 26;

    std::array<TrieNode*, ALPHABET_SIZE> children;
    bool isEndOfWord;

    // Constructor: initializes all child pointers to nullptr and flag to false
    TrieNode();

    // Destructor: recursively deletes all allocated child nodes (RAII)
    ~TrieNode();

    // Disable copy semantics to prevent double free and accidental slicing
    TrieNode(const TrieNode&) = delete;
    TrieNode& operator=(const TrieNode&) = delete;

    // Allow move semantics
    TrieNode(TrieNode&& other) noexcept;
    TrieNode& operator=(TrieNode&& other) noexcept;

    // Helper utilities for character mapping ('a'-'z')
    static int charToIndex(char c);
    static char indexToChar(int index);
};

#endif // TRIENODE_H

