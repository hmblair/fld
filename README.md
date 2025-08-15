## Overview

`fld` (Fast Library Design) is a program for the preparation of sequence libraries for MaP-seq experiments. It features
* Easy preprocessing and fast execution,
* Precise control of GC and GU content,
* Robust checks for barcode Hamming distances.

# Installation

To build, you will need `cmake`. Installation can be done by simply cloning the repository and running the installation script.
```
git clone https://github.com/hmblair/fld
cd fld
./configure
```
Don't forget to add the `./bin` directory to your path.

## Additional Dependencies

If you are running the entire pipeline, including barcode rebalancing and coverage-based sorting, you will also need
* `csvstack`,
* `rn-coverage`.

Instructions for installing `rn-coverage` can be found at https://github.com/hmblair/rn-coverage.

# Start Here

The command
```
fld-all INPUT-DIR OUTPUT-DIR CONFIG
```
will run the entire library design pipeline. All FASTA files in `INPUT-DIR` will be used as designs. The `CONFIG` file is a text file with two lines; the first line is all arguments to be passed to `fld design`, and the second is all arguments to be passed to `fld barcodes` (see below for details on what arguments are available).

For example, to pad all designs to 100nt with barcodes of stem length 13nt and restrictions on the GC and GU content, the `CONFIG` file would be
```
--max-gc 5 --max-gu 1 --pad-to 100
--max-gc 5 --max-gu 1 --length 13
```

Please note that the `--count` command for `fld-barcodes` and both `-o` commands are internally handled, and should not be passed.

## What settings should I use?

If you are unsure what settings to use, then you should set `--pad-to` for `fld design` and `--length` for `fld barcodes`, and leave the rest at their defaults.

If you get an error telling you there are not enough barcodes, then you should bump up `--max-gu` until the error goes away. You could also increase `--max-gc`, but this is more likely to cause experimental issues.

# Component Descriptions

## Preprocessing

To run `fld` manually on a set of designs, you will need to place your designs in a `.csv` file with the (exact) following columns:
```
name,sublibrary,five_const,five_padding,design,three_padding,barcode,three_const
```
If there are no existing library elements for the input designs, then you only need to fill in the name and design columns. This can be achieved with the `preprocess` subcommand:
```
fld preprocess --output library.csv [--sublibrary S] designs.fasta
```
The `--sublibrary` flag is optional and fills in the `sublibrary` column of the `.csv`. If you have multiple sublibraries, you should `preprocess` them separately and then use a program like `csvstack` to combine them together.

If your designs do have existing library elements, you will need to create this `.csv` manually.

## Library Design

Once the designs are in the proper file, the library elements can be added. A minimal command may look like
```
fld design -o out --pad-to 130 --barcode-length 10 library.csv
```
The output file will be `out.csv` with the same columns as the input, but with the appropriate columns filled in. In addition, `out.fasta` and `out.txt` will be generated as well.

That the output has the same columns as the input allows for you to append new (preprocessed) designs to an output, then run `fld design` again, and it will respect the existing library elements (e.g. it will not repeat barcodes).

The following lists all additional commands available:

`-o`: The output prefix. Required

`--pad-to`: The output prefix. Pad the design region of all sequences to this length. Required.

`--barcode-length`: The length of the stem of each barcode. A value of 0 disables barcoding. Default: 0

`--overwrite`: Overwrite any existing file.

`--five-const`: The 5' constant sequence. Default: ACTCGAGTAGAGTCGAAAA

`--five-const`: The 3' constant sequence. Default: AAAAGAAACAACAACAACAAC

`--min-stem-length`: The minimum length of the stem of a hairpin. Default: 7

`--max-stem-length`: The maximum length of the stem of a hairpin. Default: 13

`--max-au`: The maximum AU content of any stem. Default: INT_MAX

`--max-gc`: The maximum GC content of any stem. Default: 5

`--max-gu`: The maximum GU content of any stem. Default: 0

`--closing-gc`: The number of GC pairs to close each stem with. Default: 1

`--spacer`: The length of the polyA spacer used in between consecutive padding stems. Default: 2

## Barcodes

A set of unique barcodes of a given length can be generated with the command
```
fld barcodes --count N --length L --output barcodes.txt
```
The same options that are available to `design` (`--max-au`, `--max-gc`, ...) are also available here. The same Hamming distance checks are also performed.

## Inspect

The command
```
fld inspect [--sort] file
```
will list the length of all sequences in the given .fasta file. This can be useful when preprocessing as a sanity check. `--sort` will sort the results by count rather than sequence length.

## M2

Files for M2-seq can be generated with the command
```
fld m2 --output output.fasta [--all] [--overwrite] input.fasta
```
The `--all` flag specifies that all three mutants should be computed, rather than the default of computing complements only.

WARNING: `fld m2` complements both U and T to A, but only complements A to T, so if the input is RNA then the output will contain both RNA and DNA.

## Random

A random file of sequences of a given length can be generated with the command
```
fld random --count N --length L --output barcodes.txt [--fasta]
```
The `--fasta` flag will output the sequences in FASTA format with names `random_*` instead.

## Duplicate

The sequences in a FASTA file can be duplicated N times with the command
```
fld duplicate --count N --output duplicates.fasta input.fasta
```
The FASTA records will have "_x" appeneded to them for x from 0 to N-1.

# Operations

The following outlines what `fld design` will do to generate the library designs.

## Base Conversion

All bases are converted to DNA bases. If degenerate bases are present (e.g. 'N'), then they are randomly convered to one of the appropriate bases. Any other invalid characters (e.g. '_') are treated as a degenerate 'N' base.

## Padding

All sequences are padded to the length specified by the user. By default, certain heuristics are employed to ensure the formation of stems:
* The stem length is randomly sampled from {6,7,8,9}
* Each stem is closed by at least one GC pair
* The tetraloop is chosen from one of three common loops
* Multiple stems are separated by a 2bp polyA strand.

The specifics of these heuristics can be changed via various arguments. If the padding is too short to accomodate a stem, then a random sequence is sampled instead. 
## Barcoding

The same heuristics for padding are used to generate random barcodes for each sequence. An additional check is performed to guarantee that all barcodes have a Hamming distance of at least 2, to reduce the chance of a mutation in the barcode causing a mis-identification. This value (2) is fixed for now.

## Primerizing

The appropriate 5' and 3' constant regions are appended to the sequence.
