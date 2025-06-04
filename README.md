## Overview

`fld` is a program for **f**ast **l**ibrary **d**esign regarding MaP-seq experiments. It features
* Easy preprocessing and fast execution,
* Precise control of GC and GU content,
* Robust checks for barcode Hamming distances.

## Installation

To build, you will need `cmake`. Installation can be done by simply cloning the repository and running the installation script.
```
git clone https://github.com/hmblair/fld
cd fld
./configure
```
Don't forget to add the `./bin` directory to your path.

# Usage

## Preprocessing

To run `fld` on a set of designs, you will need to place your designs in a `.csv` file with the (exact) following columns:
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

Additional settings can be accessed via `fld design --help`.

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

# Utilities

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

## Barcodes

A set of unique barcodes of a given length can be generated with the command
```
fld barcodes --count N --length L --output barcodes.txt
```
The same options that are available to `design` (`--max-au`, `--max-gc`, ...) are also available here. The same Hamming distance checks are also performed.

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
