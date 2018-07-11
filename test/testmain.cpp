#include <gtest/gtest.h>
#include <iostream>

using namespace std;

GTEST_API_ int main(int argc, char* argv[])
{
    cout << "Running main() from " << __FILE__ << endl;

    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
