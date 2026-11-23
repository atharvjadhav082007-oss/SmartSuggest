#include "Dictionary.h"
#include <fstream>
#include <algorithm>
#include <cctype>
#include <iostream>

Dictionary::Dictionary() = default;

Dictionary::Dictionary(const std::string& filepath) {
    loadFromFile(filepath);
}

std::string Dictionary::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

std::string Dictionary::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return result;
}

bool Dictionary::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[Dictionary] Error: Unable to open file: " << filepath << std::endl;
        return false;
    }

    words.clear();
    std::string line;
    while (std::getline(file, line)) {
        std::string cleaned = toLower(trim(line));
        if (!cleaned.empty()) {
            words.push_back(cleaned);
        }
    }

    file.close();
    return true;
}

const std::vector<std::string>& Dictionary::getWords() const {
    return words;
}

size_t Dictionary::size() const {
    return words.size();
}

