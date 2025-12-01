#include "design_config.hpp"

void DesignConfig::validate() const {
    stem.validate();
}

void DesignConfig::validate_with_library_size(size_t library_size) const {
    validate();
    barcode.validate(library_size);
}
