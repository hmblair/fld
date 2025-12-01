#include "totxt.hpp"
#include "io/writers.hpp"

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
    _remove_if_exists(output_txt(output), overwrite);
    Library library = _from_csv(file);
    library.to_txt(output_txt(output));
}
