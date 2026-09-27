#ifndef EXECUTION_HPP
#define EXECUTION_HPP

#include <string>
#include "Farrar.hpp"

template <typename Backend>
void runBenchmark(const std::string &seq0, const std::string &seq1
                , const std::string ftype, std::ofstream &outFile)
{
    double totalTime = 0.0;
    auto start = std::chrono::high_resolution_clock::now();
    Farrar<Backend> aligner(seq0, seq1, ftype);

    int score = aligner.obtainScore();

    auto end = std::chrono::high_resolution_clock::now();
    aligner.clearData();

    std::chrono::duration<double> duration = end - start;
    double seconds = duration.count();
    totalTime += seconds;

    outFile << "Score: " << score << "\nTime: " << seconds << "s\n\n";
    std::cout << "Score: " << score << "\nTime: " << seconds << "s\n\n";
    outFile.flush();
}

inline std::string readFasta(const fs::path &fastaPath)
{
    std::ifstream file(fastaPath);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open " + fastaPath.string());
    }

    std::string line;
    std::string sequence;

    std::getline(file, line);

    while (std::getline(file, line))
    {
        sequence += line;
    }

    return sequence;
}



#endif