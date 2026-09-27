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


#ifndef EXECUTION_HPP
#define EXECUTION_HPP

#include <fstream>
#include <string>
#include "Farrar.hpp"

std::string readFasta(
    const std::string& fastaFile);

void runSelectedBackend(
    const std::string& vectype,
    const std::string& ftype,
    const std::string& seq0,
    const std::string& seq1,
    std::ofstream& outFile);

void executeBenchmark(
    const std::string& seq0File,
    const std::string& seq1File,
    const std::string& ftype,
    const std::string& vectype);

template <typename Backend>
void runBenchmark(
    const std::string& seq0,
    const std::string& seq1,
    const std::string& ftype,
    std::ofstream& outFile)
{
    auto start = std::chrono::high_resolution_clock::now();

    Farrar<Backend> aligner(seq0, seq1, ftype);

    int score = aligner.obtainScore();

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end - start;
    double seconds = duration.count();

    outFile
        << "Score: " << score
        << "\nTime: " << seconds << "s\n\n";

    std::cout
        << "Score: " << score
        << "\nTime: " << seconds << "s\n\n";

    outFile.flush();
}

#endif