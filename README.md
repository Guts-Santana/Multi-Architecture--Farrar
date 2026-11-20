# Multi-Architecture Farrar

A multi-architecture implementation of the **Farrar striped Smith-Waterman algorithm**, designed to separate the alignment algorithm from architecture-specific SIMD implementations.

The project currently supports:

* **AVX2**
* **RISC-V Vector (RVV)**
* **Arm Scalable Vector Extension (SVE)**
* 16-bit and 32-bit integer arithmetic
* Multiple RVV LMUL configurations
* Runtime SVE vector lengths
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
│   ├── execution.hpp
│   ├── AVX/
│   │   ├── AvxOps.hpp
│   │   └── AvxTraits.hpp
│   ├── RVV/
│   │   ├── RvvOps.hpp
│   │   └── RvvTraits.hpp
│   └── SVE/
│       ├── SveOps.hpp
│       └── SveTraits.hpp
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
| `SveOps` / `SveTraits` | Arm SVE implementation                       |
| `CLIParameters`        | Command-line argument parsing                |
| `execution`            | Sequence loading and backend dispatch        |

---

## 2. Architecture and Backend Configuration

The implementation separates the Farrar algorithm from SIMD operations:

```text
Farrar
  |
Backend
  +-- AVX2: Int16 / Int32
  +-- RVV: Int16 / Int32, LMUL 1/2/4/8
  +-- SVE: Int16 / Int32, runtime vector length
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

### SVE

The SVE backend provides:

```text
SveInt16
SveInt32
```

SVE vector lengths are implementation-dependent. The backend queries the
runtime lane count for each element width and uses it to determine the Farrar
segment length, rather than assuming a fixed vector width.

The target architecture is selected when configuring the project:

```text
--with-vec-arch=avx
--with-vec-arch=rvv
--with-vec-arch=sve
```

Only one architecture is enabled in a given build. The concrete vector configuration is then selected through the `--vector` command-line option.

---

## 3. Farrar Algorithm

The implementation uses the striped formulation of Smith-Waterman (local
alignment). The same kernel also runs Needleman-Wunsch (global alignment),
selected with `--alignment=nw`:

| | Smith-Waterman (`sw`, default) | Needleman-Wunsch (`nw`) |
|---|---|---|
| Floor of `H` | `0` | none |
| First row / column | `0` | a gap: `H(i,-1) = H(-1,i) = gapOpen + i * gapExt` |
| Score | best cell | last cell `H(n-1, m-1)` |

Only the floor of `H` changes inside the kernel. NW scores go far below zero,
so the lanes hold `H - b`, where `b` is a lower bound of every cell of the
matrix (a path from the corner scored with the worst score at each step). The
run is exact when `match * min(n, m) - b` fits the vector type; otherwise
`farrar` stops with an error and suggests an `Int32` backend. In practice an
`Int16` NW run is limited to sequences of a few thousand bases.

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
-m, --match=N          (default: 1)
-x, --mismatch=N       (default: -3)
-o, --gap-open=N       (default: -5)
-e, --gap-ext=N        (default: -2)
-a, --alignment=TYPE   sw (local) or nw (global) (default: sw)
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

SVE example:

```bash
./farrar \
    --vector=SveInt16 \
    --f-strategy=prefix-scan-f \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

Global alignment (Needleman-Wunsch):

```bash
./farrar \
    --alignment=nw \
    --vector=AvxInt32 \
    Sequences/30k/NC_045512.2.fasta \
    Sequences/30k/OQ983940.1.fasta
```

The NW results are saved with a `_nw` suffix, so they do not overwrite the SW
ones.

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

### SVE

On an AArch64 system with SVE support:

```bash
./configure --with-vec-arch=sve
make -j
```

The build enables:

```text
-march=armv8.2-a+sve
```

For cross-compilation, provide an AArch64 SVE-capable compiler:

```bash
./configure \
    --with-vec-arch=sve \
    --host=aarch64-linux-gnu \
    CXX=aarch64-linux-gnu-g++
make -j
```

The resulting executable must be run on an SVE-capable target.

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

Configure SVE:
    ./configure --with-vec-arch=sve

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
