#include <gtest/gtest.h>

#include "vos.h"

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    vos_init();
    return RUN_ALL_TESTS();
}
