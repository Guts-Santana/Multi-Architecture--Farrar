/*
 * Multi-Architecture Farrar
 * Copyright (C) 2026 Gustavo Santana Lima
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "execution.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "Backend.hpp"
#include "Farrar.hpp"

namespace fs = std::filesystem;

std::string readFasta(const std::string& fastaFile)
{
    std::ifstream file(fastaFile);

    if (!file.is_open())
        throw std::runtime_error(
            "Failed to open " + fastaFile);

    std::string line;
    std::string sequence;

    std::getline(file, line);

    while (std::getline(file, line))
        sequence += line;

    return sequence;
}

void runSelectedBackend(
    const std::string& vectype,
    const std::string& ftype,
    const std::string& seq0,
    const std::string& seq1,
    std::ofstream& outFile)
{
#if defined(USE_AVX)

    if (vectype == "AvxInt16")
        runBenchmark<AvxInt16>(
            seq0, seq1, ftype, outFile);

    else if (vectype == "AvxInt32")
        runBenchmark<AvxInt32>(
            seq0, seq1, ftype, outFile);

    else
        throw std::invalid_argument(
            "Invalid AVX backend: " + vectype);

#elif defined(USE_RVV)

    if (vectype == "RvvInt16M1")
        runBenchmark<RvvInt16M1>(
            seq0, seq1, ftype, outFile);

    else if (vectype == "RvvInt16M2")
        runBenchmark<RvvInt16M2>(
            seq0, seq1, ftype, outFile);

    else if (vectype == "RvvInt16M4")
        runBenchmark<RvvInt16M4>(
            seq0, seq1, ftype, outFile);

    else if (vectype == "RvvInt16M8")
        runBenchmark<RvvInt16M8>(
            seq0, seq1, ftype, outFile);

    else if (vectype == "RvvInt32M1")
        runBenchmark<RvvInt32M1>(
            seq0, seq1, ftype, outFile);

    else if (vectype == "RvvInt32M2")
        runBenchmark<RvvInt32M2>(
            seq0, seq1, ftype, outFile);

    else if (vectype == "RvvInt32M4")
        runBenchmark<RvvInt32M4>(
            seq0, seq1, ftype, outFile);

    else if (vectype == "RvvInt32M8")
        runBenchmark<RvvInt32M8>(
            seq0, seq1, ftype, outFile);

    else
        throw std::invalid_argument(
            "Invalid RVV backend: " + vectype);

#else

    throw std::runtime_error(
        "No vector backend was compiled.");

#endif
}

void executeBenchmark(
    const std::string& seq0File,
    const std::string& seq1File,
    const std::string& ftype,
    const std::string& vectype)
{
    const std::string seq0 = readFasta(seq0File);
    const std::string seq1 = readFasta(seq1File);

    fs::path folderPath = "work";

    if (!fs::exists(folderPath))
        fs::create_directories(folderPath);

    const std::string seq0Name =
        fs::path(seq0File).stem().string();

    const std::string seq1Name =
        fs::path(seq1File).stem().string();

    const std::string fileName =
        "Comparison_" +
        seq0Name + "_" +
        seq1Name + "_" +
        ftype + "_" +
        vectype + ".txt";

    const fs::path outputPath =
        folderPath / fileName;

    std::ofstream outFile(outputPath);

    if (!outFile.is_open())
        throw std::runtime_error(
            "Could not open output file: " +
            outputPath.string());

    std::cout
        << "Loaded Sequences:\n"
        << " - Seq 0 Length: " << seq0.length() << " bp\n"
        << " - Seq 1 Length: " << seq1.length() << " bp\n"
        << " - Strategy Type: " << ftype << "\n"
        << " - Vector Type: " << vectype << "\n\n";

    outFile
        << "Loaded Sequences:\n"
        << " - Seq 0 Length: " << seq0.length() << " bp\n"
        << " - Seq 1 Length: " << seq1.length() << " bp\n"
        << " - Strategy Type: " << ftype << "\n"
        << " - Vector Type: " << vectype << "\n\n";

    runSelectedBackend(
        vectype,
        ftype,
        seq0,
        seq1,
        outFile);

    std::cout
        << "Benchmark complete. Results saved to "
        << outputPath << '\n';
}