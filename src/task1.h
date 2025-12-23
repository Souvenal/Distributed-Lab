#include <filesystem>
#include <vector>

class Task1 {
public:
    Task1(const std::filesystem::path& dataDir, const std::filesystem::path& outputDir, int threads);

    auto run() -> void;

private:

    auto readText() -> void;
    auto readPatterns() -> void;

private:
    std::filesystem::path m_dataDir;
    std::filesystem::path m_outputDir;
    int m_threads;

    std::vector<std::string> m_texts;       // one text split into multiple chunks
    std::vector<std::string> m_patterns;    // multiple patterns
};