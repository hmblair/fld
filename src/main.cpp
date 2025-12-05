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
    _parent.add_subparser(test._parser);
    _parent.add_subparser(categorize._parser);
    _parent.add_subparser(sort._parser);
    _parent.add_subparser(merge._parser);
    _parent.add_subparser(pipeline._parser);
    _parent.add_subparser(prepend._parser);
    _parent.add_subparser(torna._parser);
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

bool SuperProgram::is_test() const {
    return test.used(_parent);
}

bool SuperProgram::is_categorize() const {
    return categorize.used(_parent);
}

bool SuperProgram::is_sort() const {
    return sort.used(_parent);
}

bool SuperProgram::is_merge() const {
    return merge.used(_parent);
}

bool SuperProgram::is_pipeline() const {
    return pipeline.used(_parent);
}

bool SuperProgram::is_prepend() const {
    return prepend.used(_parent);
}

bool SuperProgram::is_torna() const {
    return torna.used(_parent);
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
    } else if (is_test()) {
        return MODE::Test;
    } else if (is_categorize()) {
        return MODE::Categorize;
    } else if (is_sort()) {
        return MODE::Sort;
    } else if (is_merge()) {
        return MODE::Merge;
    } else if (is_pipeline()) {
        return MODE::Pipeline;
    } else if (is_prepend()) {
        return MODE::Prepend;
    } else if (is_torna()) {
        return MODE::ToRna;
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
                DesignConfig config;
                config.input_path = opt.file;
                config.output_prefix = opt.output;
                config.overwrite = opt.overwrite;
                config.pad_to_length = opt.pad_to;
                config.five_const = opt.five_const;
                config.three_const = opt.three_const;
                // Stem config for padding
                config.stem.min_length = opt.min_stem_length;
                config.stem.max_length = opt.max_stem_length;
                config.stem.max_au = opt.max_au;
                config.stem.max_gc = opt.max_gc;
                config.stem.max_gu = opt.max_gu;
                config.stem.closing_gc = opt.closing_gc;
                config.stem.spacer_length = opt.spacer;
                // Barcode config
                config.barcode.stem_length = opt.barcode_length;
                config.barcode.stem = config.stem;  // Use same stem config for barcodes
                _design(config);
                break;
            }

            case MODE::Barcodes: {
                BarcodesArgs& opt = parent.barcodes;
                StemConfig config;
                config.max_au = opt.max_au;
                config.max_gc = opt.max_gc;
                config.max_gu = opt.max_gu;
                config.closing_gc = opt.closing_gc;
                _barcodes(
                    opt.count,
                    opt.output,
                    opt.overwrite,
                    opt.stem_length,
                    config
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
                TxtArgs& opt = parent.txt;
                _to_txt(
                    opt.file,
                    opt.output,
                    opt.overwrite
                );
                break;
            }

            case MODE::Test: {
                return _run_tests(argv[0]);
            }

            case MODE::Categorize: {
                CategorizeArgs& opt = parent.categorize;
                _categorize(
                    opt.file,
                    opt.output,
                    opt.overwrite,
                    opt.bins
                );
                break;
            }

            case MODE::Sort: {
                SortArgs& opt = parent.sort;
                _sort(
                    opt.file,
                    opt.reads,
                    opt.output,
                    opt.overwrite,
                    opt.descending
                );
                break;
            }

            case MODE::Merge: {
                MergeArgs& opt = parent.merge;
                _merge(
                    opt.library,
                    opt.library_reads,
                    opt.barcodes,
                    opt.barcode_reads,
                    opt.output,
                    opt.overwrite
                );
                break;
            }

            case MODE::Pipeline: {
                PipelineArgs& opt = parent.pipeline;
                PipelineConfig config;
                config.inputs = opt.inputs;
                config.output_dir = opt.output;
                config.overwrite = opt.overwrite;
                config.pad_to = opt.pad_to;
                config.five_const = opt.five_const;
                config.three_const = opt.three_const;
                config.stem.min_length = opt.min_stem_length;
                config.stem.max_length = opt.max_stem_length;
                config.stem.max_au = opt.max_au;
                config.stem.max_gc = opt.max_gc;
                config.stem.max_gu = opt.max_gu;
                config.stem.closing_gc = opt.closing_gc;
                config.stem.spacer_length = opt.spacer;
                config.barcode_length = opt.barcode_length;
                config.no_barcodes = opt.no_barcodes;
                config.generate_m2 = opt.m2;
                config.predict = opt.predict;
                config.batch_size = opt.batch_size;
                _pipeline(config);
                break;
            }

            case MODE::Prepend: {
                PrependArgs& opt = parent.prepend;
                _prepend(
                    opt.file,
                    opt.output,
                    opt.sequence,
                    opt.overwrite
                );
                break;
            }

            case MODE::ToRna: {
                ToRnaArgs& opt = parent.torna;
                _to_rna(
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
