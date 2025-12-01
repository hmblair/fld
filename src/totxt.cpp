#include "totxt.hpp"


static inline std::string _PARSER_NAME = "txt";

static inline std::string _FILE_NAME = "file";
static inline std::string _FILE_HELP = "The input .csv file.";

static inline std::string _OUTPUT_NAME = "--output";
static inline std::string _OUTPUT_HELP = "The output prefix.";

static inline std::string _OVERWRITE_NAME = "--overwrite";
static inline std::string _OVERWRITE_HELP = "Overwrite any existing file.";

txtArgs::txtArgs() :
    Program(_PARSER_NAME),
    file(_parser, _FILE_NAME, _FILE_HELP),
    output(_parser, _OUTPUT_NAME, _OUTPUT_HELP),
    overwrite(_parser, _OVERWRITE_NAME, _OVERWRITE_HELP)
{}

void _to_txt(
    const std::string& file,
    const std::string& output,
    bool overwrite
) {

    // Remove any existing output files

    _remove_if_exists(_txt_name(output), overwrite);

    // Load the library from the provided .csv

    Library library = _from_csv(file);

    // Save to disk

    library.to_txt(_txt_name(output));

}
