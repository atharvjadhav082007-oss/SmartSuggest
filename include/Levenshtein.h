#ifndef LEVENSHTEIN_H
#define LEVENSHTEIN_H

#include <string>

/**
 * @class Levenshtein
 * @brief Computes Levenshtein edit distance using Pure Recursion with Branch-and-Bound pruning.
 *        Does NOT use Dynamic Programming (no DP tables, matrices, or memoization).
 */
class Levenshtein {
private:
    // Pure recursive helper function exploring branch operations (insertion, deletion, substitution)
    static int recursiveDistance(const std::string& s1, const std::string& s2,
                                 int i, int j, int currentCost, int maxThreshold);

public:
    /**
     * @brief Computes the edit distance purely recursively without dynamic programming.
     * @param s1 First string.
     * @param s2 Second string.
     * @param maxThreshold Maximum allowable distance cutoff (default 10).
     * @return Minimum edit distance (insertions, deletions, substitutions).
     */
    static int distance(const std::string& s1, const std::string& s2, int maxThreshold = 10);
};

#endif // LEVENSHTEIN_H
