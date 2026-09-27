# Multi-Architecture Farrar

A multi-architecture implementation of the **Farrar striped Smith-Waterman algorithm**, designed to separate the alignment algorithm from architecture-specific SIMD implementations.

The project currently supports:

* **AVX2**
* **RISC-V Vector (RVV)**
* 16-bit and 32-bit integer arithmetic
* Multiple RVV LMUL configurations
* Multiple `F`-vector propagation strategies

The main design goal is to keep the Farrar implementation independent of SIMD-specific intrinsics while allowing each backend to exploit its target architecture.

---

## 1. Project Structure

```text
Multi-Architecture/
├── configure.ac
├── Makefile.am
├── README.md
├── LICENSE
├── main.cpp
├── include/
│   ├── Backend.hpp
│   ├── Buffer.hpp
│   ├── CLIParameters.hpp
│   ├── Farrar.hpp
│   ├── constants.hpp
│   ├── execution.hpp
│   ├── AVX/
│   │   ├── AvxOps.hpp
│   │   └── AvxTraits.hpp
│   └── RVV/
│       ├── RvvOps.hpp
│       └── RvvTraits.hpp
├── src/
│   ├── CLIParameters.cpp
│   ├── Farrar.cpp
│   └── execution.cpp
└── Sequences/
    └── ...
```

### Main components

| Component              | Responsibility                               |
| ---------------------- | -------------------------------------------- |
| `Farrar`               | Farrar striped Smith-Waterman algorithm      |
| `Backend`              | Backend abstraction and configuration        |
| `Buffer`               | SIMD-aligned storage and vector loads/stores |
| `AvxOps` / `AvxTraits` | AVX2 implementation                          |
| `RvvOps` / `RvvTraits` | RVV implementation                           |
| `CLIParameters`        | Command-line argument parsing                |
| `execution`            | Sequence loading and backend dispatch        |

---

## 2. Architecture and Backend Configuration

The implementation separates the Farrar algorithm from SIMD operations:

```text
                    Farrar
                       │
                       ▼
                   Backend
                 ┌─────┴─────┐
                 │           │
                AVX2        RVV
                 │           │
             Int16/32    Int16/32
                            │
                         LMUL 1/2/4/8
```

`Farrar` operates through a common set of vector operations instead of directly using architecture-specific intrinsics.

For example:

```cpp
auto a = ops.load(...);
auto b = ops.load(...);
auto c = ops.add(a, b);
auto d = ops.max(c, ...);
```

The selected backend provides the corresponding implementation.

### AVX2

The AVX2 backend provides:

```text
AvxInt16
AvxInt32
```

AVX2 uses fixed 256-bit registers:

```text
int16_t → 16 elements
int32_t →  8 elements
```

### RVV

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

RVV has an implementation-dependent hardware vector length. The backend obtains the available runtime `VL` and uses it to determine the Farrar segment length:

```cpp
segLen = (sequenceLength + VL - 1) / VL;
```

This avoids assuming a fixed hardware VLEN.

The target architecture is selected when configuring the project:

```text
--with-vec-arch=avx
--with-vec-arch=rvv
```

Only one architecture is enabled in a given build. The concrete vector configuration is then selected through the `--vector` command-line option.

---

## 3. Farrar Algorithm

The implementation uses the striped formulation of Smith-Waterman.

The main data structures are:

```text
HStore
HLoad
E
F
Profile
```

For each column of the second sequence, Farrar performs:

1. Profile lookup
2. Primary DP pass
3. `HStore`/`HLoad` updates
4. `E` and `F` updates
5. `F` propagation

The available `F` propagation strategies are:

```text
prefix-scan-f
lazy-f
```

The strategy is selected through the command line.

---

## 4. Command-Line Interface

General syntax:

```text
./farrar [options] <seq0> <seq1>
```

The two FASTA files are the final positional arguments.

### Options

```text
-v, --vector=BACKEND
-f, --f-strategy=TYPE
-h, --help
```

Example:

```bash
./farrar \
    --vector=AvxInt32 \
    --f-strategy=prefix-scan-f \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

RVV example:

```bash
./farrar \
    --vector=RvvInt16M4 \
    --f-strategy=lazy-f \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

Display the available options:

```bash
./farrar --help
```

---

## 5. Compilation

The project uses **GNU Autotools**.

Generate the Autotools files when required:

```bash
autoreconf --install --force
```

### AVX2

On an x86-64 system with AVX2 support:

```bash
./configure --with-vec-arch=avx
make -j
```

The build enables:

```text
-mavx2
```

### RVV

On a RISC-V system with Vector support:

```bash
./configure --with-vec-arch=rvv
make -j
```

The build uses:

```text
-march=rv64gcv
-mabi=lp64d
```

For cross-compilation from x86-64:

```bash
./configure \
    --with-vec-arch=rvv \
    --host=riscv64-linux-gnu \
    CXX=riscv64-linux-gnu-g++

make -j
```

The resulting executable must be transferred to the RISC-V target.

### Cleaning

Remove compiled objects:

```bash
make clean
```

Remove the configured build:

```bash
make distclean
```

---

## 6. FASTA Input

The program expects two FASTA files:

```text
<seq0>
<seq1>
```

Example:

```bash
./farrar \
    --vector=AvxInt32 \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

The current implementation assumes one FASTA record per input file and is intended for the sequence datasets used in the benchmarks.

---

## 7. Execution Flow

```text
Command line
     │
     ▼
CLIParameters
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

## 8. License

This project is licensed under the GNU General Public License v3.0 or later.

See the [LICENSE](LICENSE) file for the complete license text.

---

## Quick Reference

```text
Generate Autotools files:
    autoreconf --install --force

Configure AVX2:
    ./configure --with-vec-arch=avx

Configure RVV:
    ./configure --with-vec-arch=rvv

Build:
    make -j

Clean:
    make clean

Full clean:
    make distclean

Help:
    ./farrar --help

Run:
    ./farrar [options] <seq0> <seq1>
```
