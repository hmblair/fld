#ifndef MAIN_H
#define MAIN_H

#include "utils.hpp"
#include "library.hpp"
#include "preprocess.hpp"
#include "inspect.hpp"
#include "barcodes.hpp"
#include "m2.hpp"
#include "random.hpp"
#include "duplicate.hpp"
#include "totxt.hpp"
#include "test.hpp"
#include "categorize.hpp"
#include "sort.hpp"
#include "merge.hpp"
#include "pipeline.hpp"
#include "prepend.hpp"
#include "torna.hpp"
#include "todna.hpp"
#include "diff.hpp"
#include "version.hpp"

const auto PROGRAM = "fld";
const auto VERSION = FLD_VERSION;

enum class MODE {
    Design,
    Preprocess,
    Inspect,
    Barcodes,
    M2,
    Random,
    Duplicate,
    TXT,
    Test,
    Categorize,
    Sort,
    Merge,
    Pipeline,
    Prepend,
    ToRna,
    ToDna,
    Diff
};

class SuperProgram {
private:

    Parser _parent;

public:

    DesignArgs design;
    PreprocessArgs preprocess;
    InspectArgs inspect;
    BarcodesArgs barcodes;
    M2Args m2;
    RandomArgs random;
    DuplicateArgs duplicate;
    TxtArgs txt;
    TestArgs test;
    CategorizeArgs categorize;
    SortArgs sort;
    MergeArgs merge;
    PipelineArgs pipeline;
    PrependArgs prepend;
    ToRnaArgs torna;
    ToDnaArgs todna;
    DiffArgs diff;

    SuperProgram();
    void parse(int argc, char** argv);
    MODE mode() const;

};


#endif
