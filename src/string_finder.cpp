#include "string_finder.h"

#include <tbb/parallel_for.h>

#include <algorithm>
#include <print>
#include <chrono>

namespace
{

/**
 * Check if b is a's prefix
 */
auto hasPrexfix(const std::string& a, const std::string& b) -> bool
{
    const auto& pair = std::mismatch(a.begin(), a.end(), b.begin());
    return pair.second == b.end();
}

auto longestCommonSuffix(const std::string& a, const std::string& b) -> int
{
    int len = 0;

    while (len < a.size() && len < b.size()) {
        if (a[a.size() - len - 1] != b[b.size() - len - 1]) {
            break;
        }

        ++len;
    }

    return len;
}

}   // namespace

StringFinder::StringFinder(const std::string& pattern):
    m_pattern(pattern),
    m_goodSuffixSkip(pattern.size())
{
    // last is the last index of the pattern
    int last = pattern.size() - 1;

    // Create the bad char skip table
    std::fill(m_badCharSkip.begin(), m_badCharSkip.end(), pattern.size());

    // not calculating the last byte, because the distance to move should be above 0
    for (int i = 0; i < last; i++) {
        m_badCharSkip[static_cast<unsigned char>(pattern[i])] = last - i;
    }

    // Create the good suffix skip table
    int lastPrefix = last;
    for (int i = last; i >= 0; --i) {
        // keep updating the matched suffix
        if (hasPrexfix(pattern, pattern.substr(i + 1))) {
            lastPrefix = i + 1;
        }

        // first, move to match the tail of the pattern, last - i
        m_goodSuffixSkip.at(i) = lastPrefix + last - i;
    }

    for (int i = 0; i < last; ++i) {
        int lenSuffix = longestCommonSuffix(pattern, pattern.substr(1, i));
        if (pattern.at(i - lenSuffix) != pattern.at(last - lenSuffix)) {
            m_goodSuffixSkip[last - lenSuffix] = lenSuffix + last - i;
        }
    }
}


auto StringFinder::next(const std::string& text, int start) const -> int
{
    int i = start + m_pattern.size() - 1;

    while (i < text.size()) {
        int j = m_pattern.size() - 1;
        while (j >= 0 && static_cast<unsigned char>(text.at(i)) == static_cast<unsigned char>(m_pattern.at(j))) {
            --i;
            --j;
        }

        if (j < 0) {
            return i + 1;
        }

        i += std::max(m_badCharSkip.at(static_cast<unsigned char>(text.at(i))), m_goodSuffixSkip.at(j));
    }

    return -1;
}

auto StringFinder::find(const std::string& text) const -> std::vector<int>
{
    std::vector<int> result;
    int pos = 0;
    while (pos < text.size() && (pos = next(text, pos)) != -1) {
        result.emplace_back(pos);
        pos += m_pattern.size();
    }

    return result;
}

auto StringFinder::findParallel(const std::string& text, int threads) const -> std::vector<int>
{
    size_t chunkSize = text.size() / threads;
    std::vector<std::vector<int>> localResults(threads);

    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    auto start = high_resolution_clock::now();

    tbb::parallel_for(tbb::blocked_range<size_t>(0, threads),
        [&](const tbb::blocked_range<size_t>& range) {
            for (size_t threadId = range.begin(); threadId != range.end(); ++threadId) {
                size_t startPos = threadId * chunkSize;
                size_t endPos = (threadId == threads - 1) ? text.size() : (threadId + 1) * chunkSize;
                
                size_t searchStart = startPos;
                size_t searchEnd = std::min(endPos + m_pattern.size() - 1, text.size());
                
                int pos = searchStart;
                while (true) {
                    pos = next(text, pos);
                    if (pos == -1 || pos >= searchEnd) {
                        break;
                    }

                    localResults[threadId].emplace_back(pos);
                    pos += m_pattern.size();
                }
            }
        });
    
    auto end = high_resolution_clock::now();
    std::println("Scanning {} characters took {} ms", text.size(), duration_cast<milliseconds>(end - start));

    std::vector<int> result;
    for (const auto& localResult : localResults) {
        result.insert(result.end(), localResult.begin(), localResult.end());
    }
    
    std::sort(result.begin(), result.end());
    return result;
}