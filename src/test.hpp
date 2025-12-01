#ifndef TEST_H
#define TEST_H

#include "utils.hpp"

class TestArgs : public Program {
public:
    TestArgs();
};

int _run_tests(const char* argv0);

#endif
