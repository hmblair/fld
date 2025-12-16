# fld - Fast Library Design

A tool for designing RNA/DNA sequence libraries for MaP-seq experiments. Handles preprocessing, padding with hairpin structures, barcode generation, and optional read-count prediction.

## Installation

Requires `cmake` and a C++20 compiler.

```bash
git clone https://github.com/hmblair/fld
cd fld
./configure
```

Add `./bin` to your PATH.

For read-count prediction features, also install [rn-coverage](https://github.com/hmblair/rn-coverage).

## Quick Start

### Most Common Use Case

Design a library with padding and barcodes:

```bash
fld pipeline -o output/ --pad-to 130 --barcode-length 10 designs.fasta
```

This produces:
- `output/library.csv` - Full library with all sequence components
- `output/library.fasta` - Complete sequences in FASTA format
- `output/t7-library.fasta` - Same sequences with T7 promoter prefix (GGGAACG)

### With Read-Count Prediction

If you have `rn-coverage` installed, add `--predict` to automatically balance barcodes by predicted read counts:

```bash
fld pipeline -o output/ --pad-to 130 --barcode-length 10 --predict designs.fasta
```

## Pipeline Examples

The `pipeline` command is the main entry point. Here are common use cases:

### Example 1: Basic Library Design

```bash
fld pipeline -o output/ --pad-to 130 --barcode-length 10 designs.fasta
```

**What it does:**
1. Preprocesses input FASTA
2. Pads sequences to 130nt with hairpin structures
3. Generates unique 10nt-stem barcodes
4. Outputs library files

**Output structure:**
```
output/
├── library.csv         # Final library (all components)
├── library.fasta       # Complete sequences
├── t7-library.fasta    # With T7 prefix (GGGAACG)
└── tmp/                # Intermediate files
```

### Example 2: Multiple Input Files

```bash
fld pipeline -o output/ --pad-to 130 --barcode-length 10 lib1.fasta lib2.fasta lib3.fasta
```

Each file becomes a separate sublibrary. The `index` column restarts at 1 for each file, and `sublibrary` tracks the source.

### Example 3: With M2-seq Complements

```bash
fld pipeline -o output/ --pad-to 130 --barcode-length 10 --m2 designs.fasta
```

Generates complement sequences for M2-seq experiments before padding.

### Example 4: No Barcodes

```bash
fld pipeline -o output/ --pad-to 130 --no-barcodes designs.fasta
```

Pads sequences but skips barcode generation.

### Example 5: With Read Prediction and Balancing

```bash
fld pipeline -o output/ --pad-to 130 --barcode-length 10 --predict designs.fasta
```

**Requires:** `rn-coverage` on PATH

**What it does:**
1. All basic steps (preprocess, pad, barcode)
2. Predicts read counts for library sequences
3. Predicts read counts for barcodes
4. Merges barcodes using read-count balancing (low-read designs get high-read barcodes)
5. Predicts final read counts for merged sequences
6. Verifies begin/end columns match design positions

### Example 6: Strict GC/GU Control

```bash
fld pipeline -o output/ --pad-to 130 --barcode-length 10 \
    --max-gc 4 --max-gu 0 --closing-gc 2 designs.fasta
```

Limits GC pairs to 4 per stem, forbids GU pairs, closes each stem with 2 GC pairs.

### Example 7: Custom Constant Regions

```bash
fld pipeline -o output/ --pad-to 130 --barcode-length 10 \
    --five-const "GGGAAACCC" --three-const "AAAAAAAAAA" designs.fasta
```

## CSV Format

### Output Format

The output CSV contains these columns:

| Column | Description |
|--------|-------------|
| `index` | 1-based position in original input file |
| `name` | Sequence identifier from FASTA header |
| `sublibrary` | Source file name (for tracking) |
| `five_const` | 5' constant/primer region |
| `five_padding` | 5' padding hairpins |
| `design` | Original design sequence |
| `three_padding` | 3' padding hairpins |
| `barcode` | Barcode hairpin |
| `three_const` | 3' constant/primer region |
| `begin` | 1-based start position of design in full sequence |
| `end` | 1-based end position of design in full sequence |

The full sequence is: `five_const + five_padding + design + three_padding + barcode + three_const`

The `begin` and `end` columns indicate where the original design is located within this full sequence. Sanity check: `end - begin + 1 == len(design)`.

### Input Format (Flexible)

When using `fld design` or other commands that read CSV files, only the **sequence columns** are required:

| Required Column | Description |
|-----------------|-------------|
| `five_const` | 5' constant/primer region |
| `five_padding` | 5' padding hairpins |
| `design` | Original design sequence |
| `three_padding` | 3' padding hairpins |
| `barcode` | Barcode hairpin |
| `three_const` | 3' constant/primer region |

Optional metadata columns (`index`, `name`, `sublibrary`, `begin`, `end`) are used if present, otherwise sensible defaults are applied. Columns can appear in any order.

## Pipeline Options Reference

| Option | Default | Description |
|--------|---------|-------------|
| `-o` | (required) | Output directory |
| `--pad-to` | 130 | Target sequence length for design region |
| `--barcode-length` | 10 | Barcode stem length (0 to disable) |
| `--no-barcodes` | false | Skip barcode generation entirely |
| `--m2` | false | Generate M2-seq complement sequences |
| `--predict` | false | Run rn-coverage prediction and barcode balancing |
| `--sort-by-reads` | false | Sort output by predicted reads (default: preserve input order) |
| `--overwrite` | false | Overwrite existing output directory |
| `--five-const` | ACTCGAGTAGAGTCGAAAA | 5' constant sequence |
| `--three-const` | AAAAGAAACAACAACAACAAC | 3' constant sequence |
| `--min-stem-length` | 7 | Minimum hairpin stem length |
| `--max-stem-length` | 13 | Maximum hairpin stem length |
| `--max-gc` | 5 | Maximum GC pairs per stem |
| `--max-gu` | 0 | Maximum GU pairs per stem |
| `--closing-gc` | 1 | GC pairs to close each stem |
| `--spacer` | 2 | PolyA spacer length between stems |

## Troubleshooting

**"Not enough barcodes" error:**
Increase `--max-gu` (e.g., `--max-gu 1` or `--max-gu 2`). You can also increase `--max-gc`, but this may affect experimental results.

**rn-coverage not found:**
Install from https://github.com/hmblair/rn-coverage and ensure it's on your PATH.

---

# Other Commands

## preprocess

Convert FASTA to CSV format for manual processing:

```bash
fld preprocess -o library.csv --sublibrary mylib designs.fasta
```

## design

Run just the design step (padding + barcoding) on a CSV:

```bash
fld design -o output --pad-to 130 --barcode-length 10 library.csv
```

## merge

Manually merge barcodes with read-count balancing:

```bash
fld merge --library library.csv --library-reads lib_reads.txt \
          --barcodes barcodes.txt --barcode-reads bc_reads.txt \
          -o merged
```

## sort

Add read counts to a library CSV:

```bash
fld sort --reads reads.txt -o sorted input.csv
```

Use `--sort-by-reads` to sort by read count, `--descending` for highest first.

## barcodes

Generate standalone barcodes:

```bash
fld barcodes --count 1000 --length 10 -o barcodes.txt
```

## inspect

Check sequence lengths in FASTA files:

```bash
fld inspect designs.fasta
fld inspect --sort designs.fasta  # Sort by count
```

## m2

Generate M2-seq complement sequences:

```bash
fld m2 -o output.fasta input.fasta
fld m2 -o output.fasta --all input.fasta  # All three mutants
```

## prepend

Add a prefix to all sequences:

```bash
fld prepend --sequence GGGAACG -o output.fasta input.fasta
```

## to-rna / to-dna

Convert between DNA and RNA:

```bash
fld to-rna -o rna.fasta dna.fasta  # T → U
fld to-dna -o dna.fasta rna.fasta  # U → T
```

## txt

Convert library CSV to plain text (one sequence per line):

```bash
fld txt -o output library.csv
```

## diff

Compare two FASTA files:

```bash
fld diff file1.fasta file2.fasta
```

## duplicate

Duplicate sequences N times:

```bash
fld duplicate --count 3 -o output.fasta input.fasta
```

## random

Generate random sequences:

```bash
fld random --count 100 --length 50 -o random.txt
fld random --count 100 --length 50 -o random.fasta --fasta
```

## categorize

Split FASTA by sequence length:

```bash
fld categorize -o output_dir/ --bins 130 240 500 input.fasta
```

## test

Run the test suite:

```bash
fld test
```

---

# How It Works

## Sequence Structure

A complete construct has this structure (5' to 3'):

```
[5' const] [5' padding] [design] [3' padding] [barcode] [3' const]
```

- **5'/3' const**: Primer binding sites (constant across library)
- **5'/3' padding**: Hairpin structures to reach target length
- **design**: Your original sequence of interest
- **barcode**: Unique identifier hairpin for each sequence

## Padding Algorithm

Sequences are padded to the target length using hairpin structures:
- Stem length randomly sampled from configured range
- Each stem closed with GC pairs (configurable)
- Tetraloop chosen from common stable loops
- Multiple stems separated by polyA spacers

## Barcode Generation

Barcodes are hairpin structures with:
- Configurable stem length
- Same GC/GU constraints as padding
- Guaranteed Hamming distance ≥ 2 between all barcodes

## Read-Count Balancing

When using `--predict`, barcodes are assigned to balance coverage:
1. Sort designs by predicted reads (ascending)
2. Sort barcodes by predicted reads (descending)
3. Pair them: low-read designs get high-read barcodes

This helps equalize read coverage across the library.
