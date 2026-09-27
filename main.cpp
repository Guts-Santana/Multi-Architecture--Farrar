#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "execution.hpp"

namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
    // Expected:
    // ./farrar_benchmark seq1.fasta seq2.fasta ftype vectype

    if (argc != 5)
    {
        std::cerr
            << "Usage: " << argv[0]
            << " <fasta_file1> <fasta_file2> <ftype> <vectype>\n";

        return 1;
    }

    const std::string seq0File = argv[1];
    const std::string seq1File = argv[2];
    const std::string ftype    = argv[3];
    const std::string vectype  = argv[4];

    const std::string seq0 = readFasta(seq0File);
    const std::string seq1 = readFasta(seq1File);

    fs::path folderPath = "work";

    if (!fs::exists(folderPath))
        fs::create_directories(folderPath);

    // Extract only the FASTA filenames
    const std::string seq0Name =
        fs::path(seq0File).stem().string();

    const std::string seq1Name =
        fs::path(seq1File).stem().string();

    const std::string fileName = "Comparison_" + 
        seq0Name + "_" +
        seq1Name + "_" +
        ftype + "_" +
        vectype + ".txt";

    const fs::path outputPath = folderPath / fileName;

    std::ofstream outFile(outputPath);

    if (!outFile.is_open())
    {
        std::cerr
            << "Error: Could not open output file at "
            << outputPath << '\n';

        return 1;
    }

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

    runBenchmark<AvxInt16>(
        seq0,
        seq1,
        ftype,
        outFile
    );

    outFile.close();

    std::cout
        << "Benchmark complete. Results saved to "
        << outputPath << '\n';

    return 0;
}