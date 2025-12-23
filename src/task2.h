#pragma once

#include <vector>
#include <filesystem>

class Task2 {
public:
    Task2(const std::filesystem::path& dataDir, const std::filesystem::path& outputDir, int threads);

    auto run() -> void;

private:

    auto readFiles() -> void;
    auto readPatterns() -> void;

private:
    std::filesystem::path m_dataDir;
    std::filesystem::path m_outputDir;
    int m_threads;

    std::vector<std::filesystem::path> m_files;       // all file names
    std::vector<std::string> m_patterns;    // multiple patterns
    std::vector<std::string> m_virusNames;
    size_t m_minVirusSize;
};