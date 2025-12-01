#ifndef BARCODE_CONFIG_H
#define BARCODE_CONFIG_H

#include "stem_config.hpp"

struct BarcodeConfig {
    size_t stem_length = 0;  // 0 means barcoding disabled
    StemConfig stem;

    bool is_enabled() const { return stem_length > 0; }
    void validate(size_t library_size) const;
};

#endif
