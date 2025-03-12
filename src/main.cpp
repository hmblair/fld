#include "main.hpp"

typedef argparse::ArgumentParser Parser;
class Options {
public:
    Parser program;
    std::string file;
    std::string prefix;
    bool overwrite;
    std::string five_const;
    std::string three_const;
    int pad;
    int min_stem_length;
    int max_stem_length;
    int max_stem_au;
    int max_stem_gc;
    int max_stem_gu;
    int closing_gc;
    int barcode_stem_length;

    Options() : program("fld") { };
    void parse(int argc, char** argv) {
        program.add_argument("file")
            .required()
            .help("The input csv files.");
        program.add_argument("-o", "--output")
            .required()
            .help("The output prefix.");
        program.add_argument("--pad-to")
            .required()
            .scan<'d', int>()
            .help("Pad the design region of all sequences to this length.");
        program.add_argument("--barcode-stem-length")
            .required()
            .scan<'d', int>()
            .help("The length of the stem of each barcode.");
        program.add_argument("--overwrite")
            .default_value(false)
            .implicit_value(true)
            .help("Overwrite any existing file.");
        program.add_argument("--five-const")
            .default_value("ACTCGAGTAGAGTCGAAAA")
            .help("The 5' constant sequence.");
        program.add_argument("--three-const")
            .default_value("AAAAGAAACAACAACAACAAC")
            .help("The 3' constant sequence.");
        program.add_argument("--min-stem-length")
            .default_value(6)
            .scan<'d', int>()
            .help("The minimum length of the stem of a hairpin.");
        program.add_argument("--max-stem-length")
            .default_value(9)
            .scan<'d', int>()
            .help("The maximum length of the stem of a hairpin.");
        program.add_argument("--max-au")
            .default_value(SIZE_MAX)
            .scan<'d', int>()
            .help("The maximum AU content of any stem.");
        program.add_argument("--max-gc")
            .default_value(SIZE_MAX)
            .scan<'d', int>()
            .help("The maximum GC content of any stem.");
        program.add_argument("--max-gu")
            .default_value(0)
            .scan<'d', int>()
            .help("The maximum GU content of any stem.");
        program.add_argument("--closing-gc")
            .default_value(1)
            .scan<'d', int>()
            .help("The number of GC pairs to close each stem with.");

        program.parse_args(argc, argv);
        file = program.get<std::string>("file");
        prefix = program.get<std::string>("output");
        overwrite = program.get<bool>("overwrite");
        five_const = program.get<std::string>("five-const");
        three_const = program.get<std::string>("three-const");
        pad = program.get<int>("pad-to");
        min_stem_length = program.get<int>("min-stem-length");
        max_stem_length = program.get<int>("max-stem-length");
        max_stem_au = program.get<int>("max-au");
        max_stem_gc = program.get<int>("max-gc");
        max_stem_gu = program.get<int>("max-gu");
        closing_gc = program.get<int>("closing-gc");
        barcode_stem_length = program.get<int>("barcode-stem-length");
    }
};

#define VERSION "0.2.0"

void version() {
    std::cout << "\n     fld version " + std::string(VERSION) + "\n";
}

void divider() {
    std::cout << "   ─────────────────────────────\n";
}

int main(int argc, char** argv) {

    // Parse arguments
    Options opt;
    try {
        opt.parse(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Argument error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    version();
    divider();

    // Remove any existing output files
    try {
        _remove_if_exists_all(opt.prefix, opt.overwrite);
    } catch (const std::exception& e) {
        std::cerr << "Error with the output files: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    // Load the library from the provided .csv
    Library library;
    try {
        library = _from_csv(opt.file);
    } catch (const std::exception& e) {
        std::cerr << "Error loading the library: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    // Add all desired library elements
    try {
        _add_library_elements(
            library,
            opt.pad,
            opt.barcode_stem_length,
            opt.min_stem_length,
            opt.max_stem_length,
            opt.max_stem_au,
            opt.max_stem_gc,
            opt.max_stem_gu,
            opt.closing_gc,
            opt.five_const,
            opt.three_const
        );
    } catch (const std::exception& e) {
        std::cerr << "Error processing the library: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    // Save to disk
    try {
        library.save(opt.prefix);
    } catch (const std::exception& e) {
        std::cerr << "Error saving the library: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}
