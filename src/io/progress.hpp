#ifndef PROGRESS_H
#define PROGRESS_H

#include <cstddef>
#include <string>
#include <indicators/block_progress_bar.hpp>

class ProgressBar {
public:
    explicit ProgressBar(const std::string& label);

    void update(size_t current, size_t total);
    void complete();

private:
    indicators::BlockProgressBar _bar;
};

#endif
