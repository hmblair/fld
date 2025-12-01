#include "progress.hpp"

using namespace indicators;

ProgressBar::ProgressBar(const std::string& label) :
    _bar{
        option::BarWidth{30},
        option::Start{"["},
        option::End{"]"},
        option::PrefixText{label},
        option::ForegroundColor{Color::white},
        option::FontStyles{std::vector<FontStyle>{FontStyle::bold}}
    } {}

void ProgressBar::update(size_t current, size_t total) {
    _bar.set_progress((current * 100) / total);
}

void ProgressBar::complete() {
    _bar.set_progress(100);
}
