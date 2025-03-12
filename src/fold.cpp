#include "fold.hpp"

static inline size_t _stem_length(size_t length, size_t loop_length) {
    return (length - loop_length) / 2;
}

static inline char* _allocate_structure(const std::string& sequence) {
    char *structure = (char *)malloc((sequence.length() + 1) * sizeof(char));
    if (structure == nullptr) {
        throw std::runtime_error("Structure memory allocation failed.");
    }
    return structure;
}

std::string _fold(const std::string& sequence) {
    char* c_structure = _allocate_structure(sequence);
    (void)fold(sequence.c_str(), c_structure);
    std::string structure(c_structure, sequence.length());
    free(c_structure);
    return structure;
}

static inline void _compute_pf(const std::string& sequence) {
    char* c_structure = _allocate_structure(sequence);
    (void)pf_fold(sequence.c_str(), c_structure);
    free(c_structure);
}

class BPP {
public:
    BPP(const std::string& sequence);
    ~BPP();
    double prob(size_t ix, size_t jx) const;
    double marginal(size_t ix) const;

private:
    double* _bpp;
    int* _iindx;
    size_t _length;
};

BPP::BPP(const std::string& sequence) : _bpp(nullptr), _iindx(nullptr) {
    _compute_pf(sequence);
    _bpp = export_bppm();
    _iindx = get_iindx(sequence.length());
    _length = sequence.length();
}

BPP::~BPP() {
    free_pf_arrays();
    _bpp = nullptr;
    _iindx = nullptr;
}

double BPP::prob(size_t ix, size_t jx) const {
    // Vienna arrays are 1-indexed
    ix++; jx++;
    if (jx >= ix) {
        return _bpp[_iindx[ix] - jx];
    }
    return _bpp[_iindx[jx] - ix];
}

double BPP::marginal(size_t ix) const {
    double p = 0;
    for (size_t jx = 0; jx < _length; jx++) {
        p += prob(ix, jx);
    }
    return p;
}

static inline double _score_unpaired_bpp(
    const BPP& bpp,
    size_t start,
    size_t end
) {
    double score = 0;
    for (size_t ix = start; ix < end; ix++) {
        score += 1 - bpp.marginal(ix);
    }
    return score / (end - start);
}

static inline double _score_hairpin_bpp(
    const BPP& bpp,
    size_t start,
    size_t end,
    size_t loop_length
) {
    double score = 0;
    size_t stem_length = _stem_length(end - start, loop_length);

    for (size_t ix = 0; ix < stem_length; ix++) {
        size_t fivep_ix = start + ix;
        size_t threep_ix = end - ix - 1;
        score += 2 * bpp.prob(fivep_ix, threep_ix);
    }

    for (size_t ix = 0; ix < loop_length; ix++) {
        size_t loop_ix = start + stem_length + ix;
        score += 1 - bpp.marginal(loop_ix);
    }

    return score / (end - start);
}

static inline double _score_hairpin(
    const std::string& sequence,
    size_t start,
    size_t end,
    size_t loop_length
) {
    BPP bpp(sequence);
    return _score_hairpin_bpp(bpp, start, end, loop_length);
}

static inline double _score_unpaired(
    const std::string& sequence,
    size_t start,
    size_t end
) {
    BPP bpp(sequence);
    return _score_unpaired_bpp(bpp, start, end);
}

double _score_unpaired(
    const std::string& sequence,
    const std::string& unpaired
) {
    std::string tmp = sequence + unpaired;
    return _score_unpaired(
        tmp,
        sequence.length(),
        sequence.length() + unpaired.length()
    );
}

double _score_hairpin(
    const std::string& sequence,
    const std::string& hairpin,
    size_t loop_length
) {
    std::string tmp = sequence + hairpin;
    return _score_hairpin(
        tmp,
        sequence.length(),
        sequence.length() + hairpin.length(),
        loop_length
    );
}

