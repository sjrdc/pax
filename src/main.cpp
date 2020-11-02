#include "pax.h"

#include <gtest/gtest.h>

TEST(Pax, CanParseIntegralValueArg)
{
    pax::command_line cmd("cmd");

    auto arg = cmd.add_value_argument<int>("some integer")
	.set_tag("-i")
	.set_long_tag("--integer");

    std::vector<std::string> args = {"piet", "-i", "5"};
    cmd.parse(args);

    EXPECT_EQ(5, arg.get_value());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
