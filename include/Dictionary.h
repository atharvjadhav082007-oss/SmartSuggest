#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <string>
#include <vector>

/**
 * @class Dictionary
 * @brief Demonstrates Encapsulation by hiding word storage and normalization logic.
 *
 * Words are loaded from an external file, trimmed of surrounding whitespace,
 * converted to lowercase, and stored in a private std::vector.
 * External consumers access the data only through the const getWords() method.
 */
class Dictionary {
private:
    std::vector<std::string> words; ///< Encapsulated private word list

    // Helper functions for string sanitization
    static std::string trim(const std::string& str);
    static std::string toLower(const std::string& str);

public:
    // Constructors
    Dictionary();
    explicit Dictionary(const std::string& filepath);

    // Destructor
    ~Dictionary() = default;

    /**
     * @brief Loads words from a file (one word per line), trims and lowercases them.
     * @param filepath Path to the words dictionary file.
     * @return true if successfully opened and loaded, false otherwise.
     */
    bool loadFromFile(const std::string& filepath);

    /**
     * @brief Accessor for loaded words (const reference ensures callers cannot modify internal state).
     * @return Const reference to internal words vector.
     */
    const std::vector<std::string>& getWords() const;

    /**
     * @brief Returns total number of words in the dictionary.
     */
    size_t size() const;
};

#endif // DICTIONARY_H

