#include "pax.h"

#include <gtest/gtest.h>

TEST(piet, jan)
{
    std::cout << "Hello, world!\n";
    
    pax::command_line cmd("piet");
    cmd.add_flag_argument("piet");
    cmd.add_value_argument<int>("some integer")
	.set_tag("-i")
	.set_long_tag("--integer");
    cmd.print_help(std::cout);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
