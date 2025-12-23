#include <CLI/CLI.hpp>

#include "task1.h"
#include "task2.h"

#include <print>

auto main(int argc, char* argv[]) -> int {
    CLI::App app;

    int threads;
    app.add_option("-t,--threads", threads, "Number of threads")->default_val(1);

    // std::string textFile;
    // app.add_option("--text", textFile, "Text filename");

    // std::string patternFile;
    // app.add_option("--pattern", patternFile, "Pattern filename");

    std::string dataPath;
    app.add_option("-d,--data", dataPath, "Data path")->required();

    std::string outputDir;
    app.add_option("-o,--output", outputDir, "Output directory")->default_val(".");

    CLI11_PARSE(app, argc, argv);

    std::println("==========Task 1 Begin==========");

    auto task1DataDir = std::filesystem::path(dataPath) / "document_retrieval";
    Task1 task1(task1DataDir, outputDir, threads);
    task1.run();
    std::println("===========Task 1 End===========");

    std::println();
    
    std::println("==========Task 2 Begin==========");

    auto task2DataDir = std::filesystem::path(dataPath) / "software_antivirus";
    Task2 task2(task2DataDir, outputDir, threads);
    task2.run();
    
    std::println("===========Task 2 End===========");
}