#ifndef MAIN_H
#define MAIN_H

#include "utils.hpp"
#include "library.hpp"
#include "preprocess.hpp"
#include "inspect.hpp"
#include "barcodes.hpp"
#include "m2.hpp"

const auto PROGRAM = "fld";
const auto VERSION = "0.2.0";

enum class MODE {
    Design,
    Preprocess,
    Inspect,
    Barcodes,
    M2
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

    SuperProgram();
    void parse(int argc, char** argv);
    bool is_design() const;
    bool is_preprocess() const;
    bool is_inspect() const;
    bool is_barcodes() const;
    bool is_m2() const;
    MODE mode() const;
};


#endif
