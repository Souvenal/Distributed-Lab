#pragma once

#include <string>
#include <array>
#include <vector>
#include <filesystem>

struct StringFinder {
public:
    StringFinder(const std::string& pattern);

    auto next(const std::string& text, int start = 0) const -> int;

    auto find(const std::string& text) const -> std::vector<int>;

    auto findParallel(const std::string& text, int threads) const -> std::vector<int>;

private:
    std::string             m_pattern;
    std::array<int, 256>    m_badCharSkip;
    std::vector<int>        m_goodSuffixSkip;
};