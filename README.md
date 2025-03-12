## Overview
`fld` is a program for *f*ast *l*ibrary *d*esign regrading MaP-seq experiments. It features
* Fast, compiled C++ execution
* Precise control of GC and GU content
* Barcode Hamming distance checks

## Installation

To instal, simply clone the repository and run the installation script.
```
git clone https://github.com/hmblair/fld
cd fld
./configure
```
Don't forget to add the `./bin` directory to your path.

## Usage

To run, you will need a `.csv` file with the following headers:

```
Name,Sublibrary,5' Constant,5' Padding,Design,3' Padding,Barcode,3' Constant
```

If all you have are designs, then you only need to fill in the Name, Sublibrary, and Design Region columns. For now, such a file must be created manually.

This file can then be processed by calling
```
fld -o out --pad-to 130 --barcode-stem-length 10 library.csv
```
Additional settings can be accessed via `fld --help`.

The output file will be `out.csv` with the same headers as the input, but with the appropriate columns filled in. In addition, `out.fasta` and `out.txt` will be generated as well.
