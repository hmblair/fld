#include "main.hpp"

SuperProgram::SuperProgram() : _parent(PROGRAM, VERSION) {
    _parent.add_subparser(design._parser);
    _parent.add_subparser(preprocess._parser);
    _parent.add_subparser(inspect._parser);
    _parent.add_subparser(barcodes._parser);
    _parent.add_subparser(m2._parser);
    _parent.add_subparser(random._parser);
    _parent.add_subparser(duplicate._parser);
    _parent.add_subparser(txt._parser);
};
void SuperProgram::parse(int argc, char** argv) {
    _parent.parse_args(argc, argv);
}

bool SuperProgram::is_design() const {
    return design.used(_parent);
}

bool SuperProgram::is_preprocess() const {
    return preprocess.used(_parent);
}

bool SuperProgram::is_inspect() const {
    return inspect.used(_parent);
}

bool SuperProgram::is_barcodes() const {
    return barcodes.used(_parent);
}

bool SuperProgram::is_m2() const {
    return m2.used(_parent);
}

bool SuperProgram::is_random() const {
    return random.used(_parent);
}

bool SuperProgram::is_duplicate() const {
    return duplicate.used(_parent);
}

bool SuperProgram::is_txt() const {
    return txt.used(_parent);
}

MODE SuperProgram::mode() const {
    if (is_design()) {
        return MODE::Design;
    } else if (is_preprocess()) {
        return MODE::Preprocess;
    } else if (is_inspect()) {
        return MODE::Inspect;
    } else if (is_barcodes()) {
        return MODE::Barcodes;
    } else if (is_m2()) {
        return MODE::M2;
    } else if (is_random()) {
        return MODE::Random;
    } else if (is_duplicate()) {
        return MODE::Duplicate;
    } else if (is_txt()) {
        return MODE::TXT;
    } else {
        throw std::runtime_error("Unknown subcommand.");
    }
}

int main(int argc, char** argv) {

    // Parse arguments
    SuperProgram parent;
    try {
        parent.parse(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Argument error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    try {

        switch (parent.mode()) {

            case MODE::Inspect: {
                InspectArgs& opt = parent.inspect;
                _inspect(
                    opt.files,
                    opt.sort
                );
                break;
            }

            case MODE::Preprocess: {
                PreprocessArgs& opt = parent.preprocess;
                _preprocess(
                    opt.file,
                    opt.output,
                    opt.overwrite,
                    opt.sublibrary
                );
                break;
            }

            case MODE::Design: {
                DesignArgs& opt = parent.design;
                _design(
                    opt.file,
                    opt.output,
                    opt.overwrite,
                    opt.pad_to,
                    opt.barcode_length,
                    opt.min_stem_length,
                    opt.max_stem_length,
                    opt.max_au,
                    opt.max_gc,
                    opt.max_gu,
                    opt.closing_gc,
                    opt.spacer,
                    opt.five_const,
                    opt.three_const
                );
                break;
            }

            case MODE::Barcodes: {
                BarcodesArgs& opt = parent.barcodes;
                _barcodes(
                    opt.count,
                    opt.output,
                    opt.overwrite,
                    opt.stem_length,
                    opt.max_au,
                    opt.max_gc,
                    opt.max_gu,
                    opt.closing_gc
                );
                break;
            }

            case MODE::M2: {
                M2Args& opt = parent.m2;
                _m2(
                    opt.input,
                    opt.output,
                    opt.all,
                    opt.overwrite
                );
                break;
            }

            case MODE::Random: {
                RandomArgs& opt = parent.random;
                _random(
                    opt.output,
                    opt.overwrite,
                    opt.count,
                    opt.length,
                    opt.fasta
                );
                break;
            }

            case MODE::Duplicate: {
                DuplicateArgs& opt = parent.duplicate;
                _duplicate(
                    opt.input,
                    opt.output,
                    opt.overwrite,
                    opt.count
                );
                break;
            }

            case MODE::TXT: {
                txtArgs& opt = parent.txt;
                _to_txt(
                    opt.file,
                    opt.output,
                    opt.overwrite
                );
                break;
            }

        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}
