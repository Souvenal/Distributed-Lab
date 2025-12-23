#include "task1.h"

#include "string_finder.h"

#include <tbb/parallel_for.h>

#include <fstream>
#include <print>
#include <chrono>

Task1::Task1(const std::filesystem::path& dataDir, const std::filesystem::path& outputDir, int threads):
    m_dataDir(dataDir), m_outputDir(outputDir),
    m_threads(threads)
{ }

auto Task1::run() -> void
{
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;

    auto start = high_resolution_clock::now();

    readPatterns();

    std::vector<StringFinder> finders;
    for (const auto& pattern: m_patterns) {
        finders.emplace_back(pattern);
    }

    // std::vector<std::vector<int>> results;
    // results.resize(m_patterns.size());

    std::filesystem::path documentPath = m_dataDir / "document.txt";
    std::ifstream file(documentPath, std::ios::binary);
    if (!file.is_open()) {
        std::println(stderr, "Failed to open file: {}", documentPath.string());
        return;
    }

    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();

    std::streamsize chunkSize = fileSize / m_threads;
    
    std::vector<std::vector<std::vector<int>>> threadResults(m_threads);
    for (auto& threadResult : threadResults) {
        threadResult.resize(m_patterns.size());
    }
    std::vector<int> newLinesRemoved(m_threads, 0);
    tbb::parallel_for(0, m_threads, [&](int threadId) {
        std::streamsize start = threadId * chunkSize;
        std::streamsize end = (threadId == m_threads - 1) ? fileSize : (threadId + 1) * chunkSize;
        
        std::ifstream chunkFile(documentPath, std::ios::binary);
        chunkFile.seekg(start);
        
        std::streamsize actualChunkSize = end - start;
        std::string buffer;
        buffer.resize(actualChunkSize);
        
        chunkFile.read(buffer.data(), actualChunkSize);
        std::streamsize bytesRead = chunkFile.gcount();
        chunkFile.close();
        buffer.resize(bytesRead);
        
        buffer.erase(std::remove(buffer.begin(), buffer.end(), '\n'), buffer.end());
        newLinesRemoved[threadId] = bytesRead - buffer.size();
        
        for (size_t i = 0; i < finders.size(); ++i) {
            auto foundPositions = finders[i].find(buffer);
            
            for (int pos : foundPositions) {
                threadResults[threadId][i].emplace_back(start + pos);
            }
        }
    });

    std::vector<std::vector<int>> results(m_patterns.size());

    // for (size_t i = 0; i < m_patterns.size(); ++i) {
    //     for (int j = 1; j < m_threads; ++j) {
    //         std::streamsize start = j * chunkSize - m_patterns[i].size() + 1;
    //         std::streamsize end = std::min(
    //             static_cast<std::streamsize>(j * chunkSize + m_patterns[i].size() - 1),
    //             fileSize);
    //         file.seekg(start);

    //         std::string buffer;
    //         buffer.resize(end - start);
    //         file.read(buffer.data(), end - start);
    //         buffer.erase(std::remove(buffer.begin(), buffer.end(), '\n'), buffer.end());

    //         int pos = finders[i].next(buffer, 0);
    //         if (pos != -1 && pos < m_patterns[i].size()) {
    //             results[i].emplace_back(start);
    //         }
    //     }
    // }

    file.close();

    for (size_t patternIdx = 0; patternIdx < m_patterns.size(); ++patternIdx) {
        int offset = 0;
        for (int threadId = 0; threadId < m_threads; ++threadId) {
            // results[patternIdx].insert(results[patternIdx].end(),
            //                           threadResults[threadId][patternIdx].begin(),
            //                           threadResults[threadId][patternIdx].end());
            for (int pos : threadResults[threadId][patternIdx]) {
                results[patternIdx].emplace_back(pos + offset);
            }

            offset += newLinesRemoved[threadId];
        }
    }

    auto end = high_resolution_clock::now();
    std::println("Took: {}ms", duration_cast<milliseconds>(end - start).count());

    std::filesystem::path resultPath = m_outputDir / "result_document.txt";
    std::filesystem::create_directories(resultPath.parent_path());
    std::ofstream resultFile(resultPath);
    // for (size_t i = 0; i < results.size(); ++i) {
    //     std::print(resultFile, "{}", results[i].size());
    //     for (int pos : results[i]) {
    //         std::print(resultFile, " {}", pos);
    //     }
    //     std::println(resultFile);
    // }
    for (size_t i = 0; i < results.size(); ++i) {
        resultFile << results[i].size();
        for (int pos : results[i]) {
            resultFile << " " << pos;
        }
        resultFile << "\n";
    }
}

auto Task1::readPatterns() -> void
{
    std::filesystem::path patternsPath = m_dataDir / "target.txt";
    std::ifstream file(patternsPath);

    if (!file.is_open()) {
        std::println(stderr, "Failed to open file: {}", patternsPath.string());
        return;
    }

    std::string pattern;
    while (!file.eof()) {
        std::getline(file, pattern);
        if (!pattern.empty() && !file.fail()) {
            m_patterns.emplace_back(pattern);
        }
    }

    file.close();
}