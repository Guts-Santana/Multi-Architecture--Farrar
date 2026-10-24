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


#include "CLIParameters.hpp"

CLIParametersOptions parseArguments(int argc, char* argv[])
{
    CLIParametersOptions options;

#ifdef USE_RVV
    options.vector = "RvvInt16M1";
#elif defined(USE_AVX)
    options.vector = "AvxInt16";
#else
#error "No vector backend configured"
#endif

    options.fStrategy = "prefix-scan-f";

    static struct option longOptions[] = {
        {"vector",     required_argument, nullptr, 'v'},
        {"f-strategy", required_argument, nullptr, 'f'},
        {"match",      required_argument, nullptr, 'm'},
        {"mismatch",   required_argument, nullptr, 'x'},
        {"gap-open",   required_argument, nullptr, 'o'},
        {"gap-ext",    required_argument, nullptr, 'e'},
        {"help",       no_argument,       nullptr, 'h'},
        {nullptr,      0,                 nullptr,  0}
    };

    int c;

    while ((c = getopt_long(
        argc,
        argv,
        "v:f:m:x:o:e:h",
        longOptions,
        nullptr
    )) != -1) {

        switch (c) {

        case 'v':
            options.vector = optarg;
            break;

        case 'f':
            options.fStrategy = optarg;
            break;

        case 'm':
            options.scoring.match = std::stoi(optarg);
            break;

        case 'x':
            options.scoring.mismatch = std::stoi(optarg);
            break;

        case 'o':
            options.scoring.gapOpen = std::stoi(optarg);
            break;

        case 'e':
            options.scoring.gapExt = std::stoi(optarg);
            break;

        case 'h':
            printHelp(argv[0]);
            std::exit(0);

        default:
            printHelp(argv[0]);
            throw std::runtime_error(
                "Invalid command-line arguments"
            );
        }
    }

    if (argc - optind != 2) {
        printHelp(argv[0]);

        throw std::runtime_error(
            "Expected two sequence files"
        );
    }

    options.seq0 = argv[optind];
    options.seq1 = argv[optind + 1];

    return options;
}

void printHelp(const char* program)
{
    std::cout
        << "Usage:\n"
        << "  " << program << " [options] <seq0> <seq1>\n\n"

        << "Arguments:\n"
        << "  <seq0>                 First FASTA sequence\n"
        << "  <seq1>                 Second FASTA sequence\n\n"

        << "Options:\n"
        << "  -v, --vector=BACKEND   Vector backend\n"
        << "  -f, --f-strategy=TYPE  F propagation strategy\n"
        << "  -m, --match=N          Match score, positive (default: 1)\n"
        << "  -x, --mismatch=N       Mismatch score, <= 0 (default: -3)\n"
        << "  -o, --gap-open=N       Gap open score, negative (default: -5)\n"
        << "  -e, --gap-ext=N        Gap extension score, negative (default: -2)\n"
        << "  -h, --help             Show this help message\n\n"

        << "Examples:\n"
        << "  " << program
        << " seq0.fasta seq1.fasta\n"

        << "  " << program
        << " --vector=RvvInt16M4"
        << " seq0.fasta seq1.fasta\n"

        << "  " << program
        << " seq0.fasta seq1.fasta"
        << " --f-strategy=lazy-f\n"

        << "  " << program
        << " --match=2 --mismatch=-3 --gap-open=-5 --gap-ext=-2"
        << " seq0.fasta seq1.fasta\n";
}