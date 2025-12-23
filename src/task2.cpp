#include "task2.h"

#include "string_finder.h"

#include <tbb/parallel_for.h>
#include <tbb/concurrent_vector.h>

#include <limits>
#include <fstream>
#include <print>
#include <chrono>

Task2::Task2(const std::filesystem::path& dataDir, const std::filesystem::path& outputDir, int threads):
    m_dataDir(dataDir), m_outputDir(outputDir),
    m_threads(threads), m_minVirusSize(std::numeric_limits<size_t>::max())
{ }

auto Task2::run() -> void
{
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;

    auto start = high_resolution_clock::now();

    readPatterns();
    readFiles();
    // std::println("Found {} files", m_files.size());

    std::vector<StringFinder> finders;
    for (const std::string& pattern : m_patterns) {
        finders.emplace_back(pattern);
    }

    tbb::concurrent_vector<std::pair<std::string, std::vector<int>>> results; // {filename, list of virusIdx}
    
    auto body = [&](const tbb::blocked_range<size_t>& range) {
        for (size_t i = range.begin(); i < range.end(); ++i) {
            const auto& filePath = m_files.at(i);
            
            std::ifstream file(filePath, std::ios::binary);
            if (!file.is_open()) {
                continue;
            }
            
            file.seekg(0, std::ios::end);
            std::streamsize fileSize = file.tellg();
            file.seekg(0, std::ios::beg);
            
            std::string content;
            content.resize(fileSize);
            file.read(content.data(), fileSize);
            file.close();
            
            std::vector<int> virusFound; // {pattern_idx, position}
            for (size_t patternIdx = 0; patternIdx < finders.size(); ++patternIdx) {
                if (fileSize < m_patterns[patternIdx].size()) {
                    continue;
                }
                auto foundPositions = finders[patternIdx].find(content);
                if (!foundPositions.empty()) {
                    virusFound.emplace_back(patternIdx);
                }
            }
            
            if (!virusFound.empty()) {
                results.emplace_back(filePath.string(), virusFound);
            }
        }
    };

    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, m_files.size()),
        body);

    auto end = high_resolution_clock::now();
    std::println("Took: {}ms", duration_cast<milliseconds>(end - start).count());

    std::filesystem::path resultPath = m_outputDir / "result_software.txt";
    std::filesystem::create_directories(resultPath.parent_path());
    std::ofstream resultFile(resultPath);
    // for (const auto& [filename, virusFound] : results) {
    //     std::print(resultFile, "{}", filename);
    //     for (const auto& virusIdx : virusFound) {
    //         std::print(resultFile, " {}", m_virusNames[virusIdx]);
    //     }
    //     std::println(resultFile);
    // }
    for (const auto& [filename, virusFound] : results) {
        resultFile << filename;
        for (const auto& virusIdx : virusFound) {
            resultFile << " " << m_virusNames[virusIdx];
        }
        resultFile << "\n";
    }
}

auto Task2::readPatterns() -> void
{
    std::filesystem::path virusPath = m_dataDir / "virus";
    if (!std::filesystem::exists(virusPath) || !std::filesystem::is_directory(virusPath)) {
        std::println(stderr, "Invalid virus directory: {}", virusPath.string());
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(virusPath)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        std::ifstream file(entry.path(), std::ios::binary);
        if (!file.is_open()) {
            std::println(stderr, "Failed to open file: {}", entry.path().string());
            continue;
        }
        
        std::string content;
        file.seekg(0, std::ios::end);
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        content.resize(fileSize);
        file.read(content.data(), fileSize);
        std::streamsize bytesRead = file.gcount();
        file.close();
        content.resize(bytesRead);

        // std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        // file.close();
        
        if (!content.empty()) {
            m_virusNames.emplace_back(entry.path().filename().string());
            m_patterns.emplace_back(content);
            m_minVirusSize = std::min(m_minVirusSize, content.size());
        }
    }
}

auto Task2::readFiles() -> void {
    auto gather = [&](this auto&& gather, const std::filesystem::path& path) -> void {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file() && entry.file_size() >= m_minVirusSize) {
                m_files.emplace_back(entry.path());
            } else if (entry.is_directory()) {
                gather(entry.path());
            }
        }
    };

    std::filesystem::path softwarePath = m_dataDir / "opencv-4.10.0";
    gather(softwarePath);
}