# Multi-Architecture Farrar

A multi-architecture implementation of the **Farrar striped Smith-Waterman algorithm**, designed to provide a common algorithmic implementation with architecture-specific SIMD backends.

The project currently supports:

* **AVX2**
* **RISC-V Vector (RVV)**
* 16-bit and 32-bit integer arithmetic
* Multiple RVV LMUL configurations
* Multiple strategies for propagating the `F` vector
* Runtime selection of the vector backend through CLI options

The main goal is to keep the Farrar algorithm independent of the underlying SIMD architecture while allowing each backend to exploit architecture-specific vector instructions.

---

## 1. Project Structure

```text
Multi-Architecture/
├── configure.ac
├── Makefile.am
├── Makefile.in
├── README.md
├── main.cpp
│
├── include/
│   ├── Backend.hpp
│   ├── Buffer.hpp
│   ├── CLIParameters.hpp
│   ├── Farrar.hpp
│   ├── constants.hpp
│   ├── execution.hpp
│   │
│   ├── AVX/
│   │   ├── AvxOps.hpp
│   │   └── AvxTraits.hpp
│   │
│   └── RVV/
│       ├── RvvOps.hpp
│       └── RvvTraits.hpp
│
├── src/
│   ├── CLIParameters.cpp
│   ├── Farrar.cpp
│   └── execution.cpp
│
└── Sequences/
    └── ...
```

### Main components

| Component       | Responsibility                                         |
| --------------- | ------------------------------------------------------ |
| `Farrar`        | Implements the Farrar striped Smith-Waterman algorithm |
| `Backend`       | Selects and exposes the SIMD backend                   |
| `Buffer`        | Manages vector-aligned data and SIMD loads/stores      |
| `AvxOps`        | AVX2 vector operations                                 |
| `AvxTraits`     | AVX2 vector and element type definitions               |
| `RvvOps`        | RISC-V Vector operations                               |
| `RvvTraits`     | RVV vector and element type definitions                |
| `CLIParameters` | Parses command-line options                            |
| `execution`     | Loads sequences and dispatches the selected backend    |
| `constants`     | Algorithmic scoring constants and configuration        |

---

# 2. Architecture

The implementation separates the Farrar algorithm from the SIMD implementation.

```text
                         ┌───────────────────┐
                         │      Farrar       │
                         │  Smith-Waterman   │
                         └─────────┬─────────┘
                                   │
                                   ▼
                         ┌───────────────────┐
                         │     Backend       │
                         └─────────┬─────────┘
                                   │
                  ┌────────────────┴────────────────┐
                  │                                 │
                  ▼                                 ▼
           ┌──────────────┐                  ┌──────────────┐
           │     AVX2     │                  │     RVV      │
           └──────┬───────┘                  └──────┬───────┘
                  │                                 │
          ┌───────┴───────┐             ┌───────────┴──────────┐
          │               │             │          │            │
          ▼               ▼             ▼          ▼            ▼
       Int16           Int32          M1         M2           M4/M8
```

The Farrar implementation does not directly use AVX or RVV intrinsics.

Instead, it operates through the backend interface.

Conceptually:

```cpp
Ops ops;

auto a = ops.load(...);
auto b = ops.load(...);

auto c = ops.add(a, b);
auto d = ops.max(c, ...);
```

The concrete backend determines how those operations are implemented.

---

# 3. Backend Types

## AVX2

The AVX2 backend provides:

```text
AvxInt16
AvxInt32
```

AVX2 uses fixed-width 256-bit registers.

Therefore:

```text
int16_t → 16 elements
int32_t → 8 elements
```

The AVX backend does not need a runtime vector length.

---

## RVV

The RVV backend provides:

```text
RvvInt16M1
RvvInt16M2
RvvInt16M4
RvvInt16M8

RvvInt32M1
RvvInt32M2
RvvInt32M4
RvvInt32M8
```

RVV differs from AVX because its vector length is determined by the hardware and the selected LMUL/element type.

The implementation therefore determines the runtime vector length and uses it to calculate the Farrar segment length.

Conceptually:

```cpp
VL = hardware_vector_length();
segLen = (sequence_length + VL - 1) / VL;
```

This avoids assuming that an RVV implementation always has a particular hardware VLEN.

---

# 4. Farrar Algorithm

The implementation uses the striped formulation of the Smith-Waterman dynamic-programming matrix.

The sequence is divided into vector-sized segments:

```text
Sequence
│
├── Segment 0
├── Segment 1
├── Segment 2
├── ...
└── Segment N
```

Each segment is processed using SIMD vectors.

The implementation maintains the main Farrar structures:

```text
HStore
HLoad
E
F
Profile
```

The vector profile stores substitution scores for the sequence symbols.

For each column of the second sequence, the algorithm performs the primary pass and then propagates the `F` values according to the selected strategy.

---

# 5. F Propagation Strategies

The project supports different strategies for propagating the `F` vector.

Currently available strategies include:

```text
prefix-scan-f
lazy-f
```

The strategy is selected through the command line.

For example:

```bash
--f-strategy=prefix-scan-f
```

or:

```bash
--f-strategy=lazy-f
```

This allows the same Farrar implementation to be evaluated using different `F` propagation approaches.

---

# 6. CLI

The executable uses named command-line options.

The general syntax is:

```text
./farrar [options] <seq0> <seq1>
```

The **two sequence files are always the final positional arguments**.

For example:

```bash
./farrar \
    --vector=AvxInt32 \
    --f-strategy=prefix-scan-f \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

This format makes experimental configurations easier to understand and reproduce.

---

## 6.1 Vector Backend

The vector backend is selected using:

```text
--vector=BACKEND
```

Examples:

```bash
--vector=AvxInt16
```

```bash
--vector=AvxInt32
```

```bash
--vector=RvvInt16M1
```

```bash
--vector=RvvInt16M4
```

```bash
--vector=RvvInt32M8
```

---

## 6.2 F Propagation Strategy

The F propagation strategy is selected using:

```text
--f-strategy=STRATEGY
```

Examples:

```bash
--f-strategy=prefix-scan-f
```

```bash
--f-strategy=lazy-f
```

---

## 6.3 Help

The available options can be displayed with:

```bash
./farrar --help
```

or:

```bash
./farrar -h
```

The expected interface is:

```text
Usage:
  ./farrar [options] <seq0> <seq1>

Arguments:
  <seq0>                 First FASTA sequence
  <seq1>                 Second FASTA sequence

Options:
  -v, --vector=BACKEND   Vector backend
  -f, --f-strategy=TYPE  F propagation strategy
  -h, --help             Show this help message
```

---

# 7. CLI Examples

## Default configuration

If defaults are configured for the selected architecture:

```bash
./farrar \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

---

## AVX Int32

```bash
./farrar \
    --vector=AvxInt32 \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

---

## AVX Int16

```bash
./farrar \
    --vector=AvxInt16 \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

---

## RVV Int16 LMUL=1

```bash
./farrar \
    --vector=RvvInt16M1 \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

---

## RVV Int16 LMUL=4

```bash
./farrar \
    --vector=RvvInt16M4 \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

---

## Selecting the F propagation strategy

```bash
./farrar \
    --vector=RvvInt16M4 \
    --f-strategy=lazy-f \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

---

# 8. Compilation

The project uses **GNU Autotools**.

The main build files are:

```text
configure.ac
Makefile.am
```

The vector architecture is selected when configuring the project.

---

## 8.1 Generate the Build System

If the project has just been cloned or the Autotools files have changed, run:

```bash
autoreconf --install --force
```

This generates the required Autotools files and auxiliary scripts.

---

# 9. Compile for AVX2

On an x86-64 machine with AVX2 support:

```bash
./configure --with-vec-arch=avx
```

Then:

```bash
make -j
```

The compiler will receive the AVX2 option:

```text
-mavx2
```

The resulting executable is:

```text
./farrar
```

You can test it with:

```bash
./farrar \
    --vector=AvxInt32 \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

---

# 10. Compile for RISC-V Vector

On a RISC-V machine with RVV support:

```bash
./configure --with-vec-arch=rvv
```

Then:

```bash
make -j
```

The RVV build uses:

```text
-march=rv64gcv
-mabi=lp64d
```

The resulting executable can be run on the RISC-V system:

```bash
./farrar \
    --vector=RvvInt16M1 \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

---

# 11. Cross Compilation for RVV

RVV binaries cannot be executed directly on an x86-64 machine.

If compiling from an x86 machine for a RISC-V target, a RISC-V cross compiler must be used.

For example:

```bash
./configure \
    --with-vec-arch=rvv \
    CXX=riscv64-linux-gnu-g++
```

Depending on the toolchain and target environment, it may also be necessary to specify the host:

```bash
./configure \
    --with-vec-arch=rvv \
    --host=riscv64-linux-gnu \
    CXX=riscv64-linux-gnu-g++
```

Then:

```bash
make -j
```

The resulting `farrar` executable must be transferred to the RISC-V target before execution.

---

# 12. Selecting the Architecture During Configuration

The architecture is selected with:

```text
--with-vec-arch=ARCH
```

Supported values are:

```text
avx
rvv
```

### AVX

```bash
./configure --with-vec-arch=avx
```

This enables:

```cpp
USE_AVX
```

### RVV

```bash
./configure --with-vec-arch=rvv
```

This enables:

```cpp
USE_RVV
```

The two architectures cannot be enabled simultaneously.

---

# 13. Clean Build

To remove compiled objects:

```bash
make clean
```

To remove generated configuration files as well:

```bash
make distclean
```

If the Autotools system itself needs to be regenerated:

```bash
autoreconf --install --force
```

Then configure again.

For example:

```bash
make distclean
autoreconf --install --force
./configure --with-vec-arch=avx
make -j
```

---

# 14. Out-of-Source Build

For cleaner development, an out-of-source build can be used.

For example:

```text
Multi-Architecture/
├── configure.ac
├── Makefile.am
├── main.cpp
├── include/
├── src/
├── Sequences/
└── build/
```

Create the build directory:

```bash
mkdir build
cd build
```

Then configure the project:

```bash
../configure --with-vec-arch=avx
```

Build:

```bash
make -j
```

The generated executable will be inside the build directory:

```text
build/farrar
```

For RVV:

```bash
../configure --with-vec-arch=rvv
make -j
```

---

# 15. Backend Selection

The compile-time configuration determines the available architecture:

```text
configure
    │
    ├── AVX
    │    └── USE_AVX
    │
    └── RVV
         └── USE_RVV
```

The CLI then selects the specific backend.

For example, an RVV build can expose:

```text
RvvInt16M1
RvvInt16M2
RvvInt16M4
RvvInt16M8
RvvInt32M1
RvvInt32M2
RvvInt32M4
RvvInt32M8
```

The architecture is therefore selected at build time, while the concrete backend configuration is selected when executing the program.

---

# 16. Buffer

`Buffer` provides storage for the vectorized Farrar data structures.

It is responsible for:

* Allocating storage
* Managing vector-sized arrays
* Loading SIMD vectors
* Storing SIMD vectors
* Managing the lifetime of the underlying data

Conceptually:

```text
Buffer
   │
   ├── memory
   ├── load()
   ├── store()
   └── size()
```

The buffer uses the backend's vector and element types.

---

# 17. SIMD Operations

The architecture-specific `Ops` classes provide the operations required by Farrar.

Typical operations include:

```text
add
sub
max
maxValue
anyBiggerElement
shift
slideup
lastElement
setVL
```

The Farrar algorithm does not need to know whether the operation is implemented using AVX2 or RVV.

For example:

```cpp
ops.add(a, b);
```

is translated by the selected backend into the appropriate architecture-specific operation.

---

# 18. RVV Runtime Vector Length

RVV does not have a fixed vector length equivalent to AVX2's 256-bit registers.

The backend determines the available vector length for the selected element type and LMUL.

For example:

```text
VLEN
  │
  ├── int16
  │
  └── int32
```

The runtime vector length is used to calculate:

```cpp
segLen = (sequenceLength + VL - 1) / VL;
```

This allows the same RVV implementation to run on processors with different hardware vector lengths without hardcoding a specific number of lanes.

---

# 19. FASTA Input

The executable expects two FASTA files:

```text
<seq0>
<seq1>
```

For example:

```bash
./farrar \
    --vector=AvxInt32 \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

The execution layer loads the sequences and passes them to the Farrar implementation.

The current implementation is intended for the sequence files used by the benchmark and assumes a single FASTA record per input file.

---

# 20. Execution Flow

The complete execution flow is:

```text
Command line
     │
     ▼
CLIParameters
     │
     ├── vector
     ├── f-strategy
     ├── seq0
     └── seq1
     │
     ▼
executeBenchmark()
     │
     ▼
runSelectedBackend()
     │
     ▼
Farrar<Backend>
     │
     ├── Build profile
     ├── Initialize matrices
     ├── Process columns
     ├── Primary pass
     └── F propagation
     │
     ▼
Alignment score
```

---

# 21. Adding a New Backend

The project is designed to allow additional SIMD architectures to be added without modifying the Farrar algorithm.

A new backend should provide:

```text
Backend
├── VecType
├── Traits
└── Ops
```

The architecture-specific operations should implement the interface required by Farrar.

For example, a future SVE backend could provide:

```text
SveInt16
SveInt32
```

while keeping the Farrar implementation unchanged.

The intended design is:

```text
Farrar
   │
   └── Backend abstraction
          │
          ├── AVX2
          ├── RVV
          └── Future SIMD backends
```

---

# 22. Correctness

The SIMD implementations should produce the same alignment score as the reference implementation.

When adding or modifying a backend, correctness should be checked before performance measurements.

Recommended validation:

```text
Reference implementation
        │
        ├── AVX16
        ├── AVX32
        ├── RVV16M1
        ├── RVV16M2
        ├── RVV16M4
        ├── RVV16M8
        ├── RVV32M1
        ├── RVV32M2
        ├── RVV32M4
        └── RVV32M8
```

All implementations should produce the same Smith-Waterman score for the same input and scoring configuration.

---

# 23. Performance Evaluation

The implementation is intended for performance evaluation across SIMD architectures.

Typical measurements include:

* Execution time
* GCUPS
* Speedup
* Vector configuration
* Integer precision
* F propagation strategy
* Sequence size

For example:

```text
Architecture   Backend       Strategy          Time
-------------------------------------------------------
AVX2           AvxInt16     prefix-scan-f     ...
AVX2           AvxInt32     prefix-scan-f     ...
RVV            RvvInt16M1   prefix-scan-f     ...
RVV            RvvInt16M4   prefix-scan-f     ...
RVV            RvvInt32M4   lazy-f            ...
```

For scientific experiments, the exact command used to generate each result should be recorded.

---

# 24. Example Benchmark

Example using AVX2:

```bash
./farrar \
    --vector=AvxInt32 \
    --f-strategy=prefix-scan-f \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

Example using RVV:

```bash
./farrar \
    --vector=RvvInt16M4 \
    --f-strategy=prefix-scan-f \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

Example comparing the two F propagation strategies:

```bash
./farrar \
    --vector=RvvInt16M4 \
    --f-strategy=prefix-scan-f \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

and:

```bash
./farrar \
    --vector=RvvInt16M4 \
    --f-strategy=lazy-f \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

---

# 25. Development Workflow

A typical development cycle is:

```bash
autoreconf --install --force

./configure --with-vec-arch=avx

make clean
make -j

./farrar \
    --vector=AvxInt32 \
    --f-strategy=prefix-scan-f \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

For RVV:

```bash
autoreconf --install --force

./configure --with-vec-arch=rvv

make clean
make -j

./farrar \
    --vector=RvvInt16M4 \
    --f-strategy=prefix-scan-f \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

---

# 26. Design Goals

The implementation follows several design goals:

1. **Architecture independence**
   The Farrar algorithm should not contain architecture-specific SIMD intrinsics.

2. **Backend specialization**
   AVX2 and RVV implementations should exploit their respective instruction sets.

3. **Runtime backend selection**
   The concrete vector configuration can be selected through the CLI.

4. **Runtime RVV vector length**
   The implementation should not assume a fixed RVV hardware VLEN.

5. **Extensibility**
   Additional SIMD architectures should be possible without rewriting Farrar.

6. **Reproducible evaluation**
   Vector backend and F propagation strategy are explicitly represented in the command line.

7. **Correctness before optimization**
   Different backends must produce equivalent alignment scores.

---

# 27. License

Add the project's license information here if applicable.

---

# 28. Quick Reference

### Generate Autotools files

```bash
autoreconf --install --force
```

### Configure AVX2

```bash
./configure --with-vec-arch=avx
```

### Configure RVV

```bash
./configure --with-vec-arch=rvv
```

### Compile

```bash
make -j
```

### Clean

```bash
make clean
```

### Full clean

```bash
make distclean
```

### Show CLI help

```bash
./farrar --help
```

### Run AVX

```bash
./farrar \
    --vector=AvxInt32 \
    --f-strategy=prefix-scan-f \
    seq0.fasta \
    seq1.fasta
```

### Run RVV

```bash
./farrar \
    --vector=RvvInt16M4 \
    --f-strategy=prefix-scan-f \
    seq0.fasta \
    seq1.fasta
```

The general command format is:

```text
./farrar [options] <seq0> <seq1>
```

## License

This project is licensed under the GNU General Public License v3.0 or later.

See the [LICENSE](LICENSE) file for the complete license text.