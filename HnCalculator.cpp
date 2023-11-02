#include <iostream>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <thread>
#include <sstream>
#include <stdexcept>
#include <chrono>

class HnCalculator {
public:
    const bool cacheResults;
    /**
     * Whether to skip permutations of the same digits
     *
     * Since whether a number is happy is determined from a sum relating to digits, permutations do not matter
     * e.g: 123, 132, 213, 231, 312 and 321 are all permutations of each other
     * Since 123 is not a happy number, neither are any of its permutations
     */
    const bool skipPermutations;
    /**
     * The base for which digits should be taken (defaults to 10, meaning denary/decimal)
     */
    const char base;
    /**
     * How many numbers should be calculated by threads (including skipped numbers)
     * In other words, the highest number calculated
     */
    std::uint64_t stopAt = UINT64_MAX;
    /**
     * Whether to output every result
     */
    bool outputResults = true;
    /**
     * How far apart milestones should be announced
     */
    std::optional<std::uint64_t> milestoneInc;

private:
    std::unordered_map<std::uint64_t,bool> cache;
    std::uint64_t nextNumber = 1;
    std::uint64_t lastMilestone = 0;
    std::mutex cacheLock;
    std::mutex nextNumberLock;

public:
    explicit HnCalculator(const bool cacheResults=true, const bool skipPermutations=true, const char base=10)
            : cacheResults(cacheResults), skipPermutations(skipPermutations), base(base) {
        if (cacheResults) {
            cache[1] = true;
            cache[4] = false;
        }
    }

    /**
     * Creates a given number of threads for calculating
     *
     * @param numThreads The number of threads to create
     * @param attachToLast Whether the calling thread should be used. If false (default), this will be a background task
     */
    void startThreads(const std::uint16_t numThreads=1, const bool attachToLast=false) {
        for (std::uint16_t i = 0; i < numThreads-attachToLast; i++) {
            std::thread(&HnCalculator::threadLoop, this).detach();
        }
        if (attachToLast) {
            threadLoop();
        }
    }

    /**
     * Determines if a given number is happy
     *
     * @param n The number which must be calculated
     * @return Whether n is happy
     */
    bool isHappy(const std::uint64_t &n) { // NOLINT(*-no-recursion)
        if (isCached(n)) {
            return cache[n];
        } else if (n == 1) {
            return true;
        } else if (n == 4) {
            return false;
        }
        std::uint64_t childNumber = sumOfDigitSquares(n);
        if (skipPermutations) {
            childNumber = sortDigits(childNumber);
        }
        const bool &happy = isHappy(childNumber);
        newResult(n,happy);
        return happy;
    }

private:
    /**
     * Iteratively calculates whether numbers are happy until stopAt is reached
     */
    void threadLoop() {
        std::uint64_t n = 0;
        while (n < stopAt) {
            isHappy(n = getNextNumber());
        }
    }

    /**
     * Gets the next number needing calculated and announces milestones
     *
     * This will skip permutations if skipPermutations is true
     *
     * @return The next number to be calculated
     */
    std::uint64_t getNextNumber() {
        nextNumberLock.lock();
        for (std::uint64_t i = nextNumber; true; i++) {
            if (!skipPermutations || areDigitsSorted(i)) {
                if (milestoneInc && i > lastMilestone+milestoneInc.value()) {
                    lastMilestone += milestoneInc.value();
                    std::stringstream msg;
                    msg << lastMilestone << " numbers calculated" << std::endl;
                    std::cout << msg.str();
                }
                nextNumber = i+1;
                nextNumberLock.unlock();
                return i;
            }
        }
    }

    /**
     * Checks if a given number has been cached
     *
     * @param n The number for which the sum of digit squares must be calculated
     * @return The sum of digit squares of n
     */
    bool isCached(const std::uint64_t &n) {
        if (!cacheResults) {
            return false;
        }
        cacheLock.lock();
        const bool cached = cache.find(n) != cache.end();
        cacheLock.unlock();
        return cached;
    }

    /**
     * Calculates the sum of the squares of the digits of a given number
     *
     * This is what inevitably determines if a number is happy
     *
     * @param n The number for which the sum of digit squares must be calculated
     * @return The sum of digit squares of the given number
     */
    constexpr std::uint64_t sumOfDigitSquares(const std::uint64_t &n) const { // NOLINT(*-no-recursion)
        if (n == 0) {
            return 0;
        }
        return (n%base)*(n%base)+sumOfDigitSquares(n/base);
    }

    /**
     * Determines if the digits of a given number are in ascending order
     *
     * This is used for skipping permutations
     *
     * @param n The number for which the digits must be sorted
     * @return Whether the digits are in ascending order
     */
    constexpr bool areDigitsSorted(std::uint64_t n) const {
        char prevDigit = base;
        while (n > 0) {
            if (n%base > prevDigit) {
                return false;
            }
            prevDigit = (char)(n%base);
            n /= base;
        }
        return true;
    }

    /**
     * Sorts the digits of a given number in ascending order
     *
     * This is used for skipping permutations
     *
     * @param n The number for which the digits must be sorted
     * @return the value of the sorted digits, can not be more than n
     */
    std::uint64_t sortDigits(std::uint64_t n) const {
        char *digits = new char[base];
        while (n != 0) {
            if (n%base != 0) {
                digits[n%base-1]++;
            }
            n /= base;
        }
        std::uint64_t result = 0;
        for (char digit = 1; digit < base; digit++) {
            for (char i = 0; i < digits[digit-1]; i++) {
                result *= base;
                result += digit;
            }
        }
        return result;
    }

    /**
     * Handles a given new result
     *
     * Outputs the given result if outputResults
     * Caches the given result if cacheResults
     *
     * @param n The number for which a result has been determined
     * @param happy The result- whether n was determined to be happy
     */
    void newResult(const std::uint64_t &n, const bool &happy) {
        if (outputResults) {
            std::stringstream msg;
            msg << n << " is" << (happy ? "" : " not") << " happy" << std::endl;
            std::cout << msg.str();
        }
        if (cacheResults) {
            cacheLock.lock();
            cache.emplace(n,happy);
            cacheLock.unlock();
        }
    }
};

/**
 * Test how long an HnCalculator takes to compute using the given number of threads
 *
 * This is useful for determining the most optimal number of threads to use on a given machine
 *
 * @param calculator Pre-configured calculator to compute
 * @param threads Number of threads to use for computation
 * @return number of ticks taken
 */
std::chrono::steady_clock::duration testThreads(HnCalculator &calculator, char threads) {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    calculator.startThreads(threads,true);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    return end-start;
}

int main() {
    HnCalculator calculator = HnCalculator();
    calculator.stopAt = 2000000000;
    calculator.outputResults = false;
    calculator.milestoneInc = 10000000;
    std::chrono::steady_clock::duration elapsedTime = testThreads(calculator, 1);
    std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count() << " milliseconds";
}
