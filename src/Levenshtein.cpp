#include "Levenshtein.h"
#include <algorithm>
#include <cmath>

/**
 * Pure Recursive Edit Distance implementation.
 *
 * NOTE: Dynamic programming (DP tables, 2D arrays, or memoization) is intentionally NOT used.
 * Instead, this uses a pure recursive tree search with branch-and-bound pruning:
 * - If remaining character difference already exceeds the max threshold, prune immediately.
 * - Matching characters advance without cost.
 * - Recursively branches on Substitution, Deletion, and Insertion.
 */
int Levenshtein::recursiveDistance(const std::string& s1, const std::string& s2,
                                   int i, int j, int currentCost, int maxThreshold) {
    int rem1 = static_cast<int>(s1.size()) - i;
    int rem2 = static_cast<int>(s2.size()) - j;

    // Prune branch if remaining length difference + current cost exceeds threshold
    if (currentCost + std::abs(rem1 - rem2) > maxThreshold) {
        return maxThreshold + 1;
    }

    // Advance identical characters without branching cost
    while (i < static_cast<int>(s1.size()) && j < static_cast<int>(s2.size()) && s1[i] == s2[j]) {
        i++;
        j++;
    }

    // Base cases: if one string is exhausted, remaining characters must be added/removed
    if (i == static_cast<int>(s1.size())) {
        return currentCost + (static_cast<int>(s2.size()) - j);
    }
    if (j == static_cast<int>(s2.size())) {
        return currentCost + (static_cast<int>(s1.size()) - i);
    }

    if (currentCost >= maxThreshold) {
        return maxThreshold + 1;
    }

    // 1. Substitution (advance both)
    int sub = recursiveDistance(s1, s2, i + 1, j + 1, currentCost + 1, maxThreshold);
    // 2. Deletion from s1 (advance i)
    int del = recursiveDistance(s1, s2, i + 1, j, currentCost + 1, std::min(maxThreshold, sub));
    // 3. Insertion into s1 (advance j)
    int ins = recursiveDistance(s1, s2, i, j + 1, currentCost + 1, std::min({maxThreshold, sub, del}));

    return std::min({sub, del, ins});
}

int Levenshtein::distance(const std::string& s1, const std::string& s2, int maxThreshold) {
    if (s1 == s2) {
        return 0;
    }
    return recursiveDistance(s1, s2, 0, 0, 0, maxThreshold);
}
