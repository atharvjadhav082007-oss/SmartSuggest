#ifndef TRIE_H
#define TRIE_H

#include "TrieNode.h"
#include <string>
#include <vector>

/**
 * @class Trie
 * @brief Prefix Tree data structure for high-performance prefix lookup and word search.
 *
 * Demonstrates:
 * - Encapsulation (hides node management, pointers, and traversal logic)
 * - Constructors and Destructors (RAII root node ownership)
 * - Copy Prevention (copy constructor and copy assignment are explicitly deleted)
 */
class Trie {
private:
    TrieNode* root; ///< Root node of the Trie

    // Recursive Depth First Search (DFS) helper to collect words with the current prefix
    void dfs(TrieNode* node, std::string& currentWord, std::vector<std::string>& results, size_t maxCount) const;

public:
    // Constructor: Allocates root TrieNode
    Trie();

    // Destructor: Frees root node, which recursively frees all descendants (RAII)
    ~Trie();

    // Explicitly delete copy constructor and copy assignment operator (Prevents shallow copying)
    Trie(const Trie&) = delete;
    Trie& operator=(const Trie&) = delete;

    // Move constructor and move assignment operator
    Trie(Trie&& other) noexcept;
    Trie& operator=(Trie&& other) noexcept;

    /**
     * @brief Inserts a word into the Trie. Converts characters to lowercase.
     * @param word The word string to insert.
     */
    void insert(const std::string& word);

    /**
     * @brief Checks if an exact word exists in the Trie.
     * @param word The word string to search.
     * @return true if the word exists, false otherwise.
     */
    bool search(const std::string& word) const;

    /**
     * @brief Retrieves words starting with a given prefix using Depth-First Search (DFS).
     * @param prefix Prefix to search for.
     * @param maxCount Maximum number of matching words to return (default 10).
     * @return Vector of matching words.
     */
    std::vector<std::string> wordsWithPrefix(const std::string& prefix, size_t maxCount = 10) const;

    /**
     * @brief Clears all words and resets the Trie.
     */
    void clear();
};

#endif // TRIE_H

