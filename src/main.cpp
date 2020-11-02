#include "pax.h"

#include <gtest/gtest.h>

TEST(Pax, CanParseIntegralValueArg)
{
    pax::command_line cmd("cmd");

    auto& arg = cmd.add_value_argument<int>("some integer")
	.set_tag("-i")
	.set_long_tag("--integer");

    constexpr auto i = 5;
    std::vector<std::string> args = {"piet", "-i", std::to_string(i)};
    cmd.parse(args);

    EXPECT_EQ(5, arg.get_value());
}

TEST(Pax, CanParseFloatingPointValueArg)
{
    pax::command_line cmd("cmd");

    auto& arg = cmd.add_value_argument<float>("some integer")
	.set_tag("-f")
	.set_long_tag("--float");

    constexpr auto f = 1.23f;
    std::vector<std::string> args = {"piet", "-f", std::to_string(f)};
    cmd.parse(args);

    EXPECT_EQ(f, arg.get_value());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
