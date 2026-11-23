#ifndef SUGGESTER_H
#define SUGGESTER_H

#include <string>
#include <vector>

/**
 * @class Suggester
 * @brief Abstract Base Class defining the contract for suggestion engines.
 *
 * Demonstrates:
 * - Abstraction: Exposes high-level suggest() interface, hiding implementation details.
 * - Polymorphism: Allows callers to treat Autocomplete and SpellChecker uniformly
 *   via Suggester* or Suggester& references.
 * - Virtual Destructor: Ensures proper cleanup of derived classes in polymorphic usage.
 */
class Suggester {
public:
    // Virtual destructor ensures proper destruction of derived instances
    virtual ~Suggester() = default;

    /**
     * @brief Pure virtual method to generate suggestions based on user input.
     * @param query The input string to provide suggestions for.
     * @return Vector of suggested words.
     */
    virtual std::vector<std::string> suggest(const std::string& query) = 0;
};

#endif // SUGGESTER_H

