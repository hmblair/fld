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
    _parent.add_subparser(todna._parser);
    _parent.add_subparser(diff._parser);
};
void SuperProgram::parse(int argc, char** argv) {
    _parent.parse_args(argc, argv);
}

MODE SuperProgram::mode() const {
    if (design.used(_parent))     return MODE::Design;
    if (preprocess.used(_parent)) return MODE::Preprocess;
    if (inspect.used(_parent))    return MODE::Inspect;
    if (barcodes.used(_parent))   return MODE::Barcodes;
    if (m2.used(_parent))         return MODE::M2;
    if (random.used(_parent))     return MODE::Random;
    if (duplicate.used(_parent))  return MODE::Duplicate;
    if (txt.used(_parent))        return MODE::TXT;
    if (test.used(_parent))       return MODE::Test;
    if (categorize.used(_parent)) return MODE::Categorize;
    if (sort.used(_parent))       return MODE::Sort;
    if (merge.used(_parent))      return MODE::Merge;
    if (pipeline.used(_parent))   return MODE::Pipeline;
    if (prepend.used(_parent))    return MODE::Prepend;
    if (torna.used(_parent))      return MODE::ToRna;
    if (todna.used(_parent))      return MODE::ToDna;
    if (diff.used(_parent))       return MODE::Diff;
    throw std::runtime_error("Unknown subcommand.");
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
                    opt.descending,
                    opt.sort_by_reads
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
                    opt.overwrite,
                    opt.sort_by_reads
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
                config.sort_by_reads = opt.sort_by_reads;
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

            case MODE::ToDna: {
                ToDnaArgs& opt = parent.todna;
                _to_dna(
                    opt.file,
                    opt.output,
                    opt.overwrite
                );
                break;
            }

            case MODE::Diff: {
                DiffArgs& opt = parent.diff;
                bool identical = _diff(opt.file1, opt.file2);
                return identical ? EXIT_SUCCESS : EXIT_FAILURE;
            }

        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}
