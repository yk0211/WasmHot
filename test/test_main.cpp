#include "test_common.h"

int run_object_tests();
int run_plugin_tests();

int g_test_failures = 0;

int main()
{
    run_object_tests();
    run_plugin_tests();

    if (g_test_failures == 0)
    {
        std::cout << "All tests passed.\n";
        return 0;
    }
    std::cerr << g_test_failures << " test(s) failed.\n";
    return 1;
}
