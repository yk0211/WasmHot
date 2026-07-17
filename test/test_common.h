#pragma once
#include <iostream>
#include <cstdlib>

extern int g_test_failures;

#define CHECK(cond) \
    do { if (!(cond)) { \
        std::cerr << "FAIL: " << #cond << " at " << __FILE__ << ":" << __LINE__ << "\n"; \
        ++g_test_failures; \
    } } while(0)
