#include "pax.h"
#include <filesystem>
#include <gtest/gtest.h>

class Pax : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    pax::command_line cmd {"cmd"};    
};

TEST_F(Pax, CanParseIntegralValueArg)
{
    auto& arg = cmd.add_value_argument<int>("some integer")
	.set_tag("-i")
	.set_long_tag("--integer");

    constexpr auto i = 5;
    std::vector<std::string> args = {"piet", "-i", std::to_string(i)};
    cmd.parse(args);

    EXPECT_EQ(5, arg.get_value());
}

TEST_F(Pax, CanStoreIntegralValueInBoundVariable)
{
    int q;
    auto& arg = cmd.add_value_argument<int>("some integer")
	.set_tag("-i")
	.set_long_tag("--integer")
	.bind(&q);

    constexpr auto i = 5;
    std::vector<std::string> args = {"piet", "-i", std::to_string(i)};
    cmd.parse(args);

    EXPECT_EQ(q, i);
}

TEST_F(Pax, CanParseFloatingPointValueArg)
{
    auto& arg = cmd.add_value_argument<float>("some float")
	.set_tag("-f")
	.set_long_tag("--float");

    constexpr auto f = 1.23f;
    std::vector<std::string> args = {"piet", "-f", std::to_string(f)};
    cmd.parse(args);

    EXPECT_FLOAT_EQ(f, arg.get_value());
}

TEST_F(Pax, FlagArgValueIsFalseByDefault)
{
    auto& arg = cmd.add_flag_argument("flag");
    EXPECT_FALSE(arg.get_value());
}

TEST_F(Pax, CanParseFlagArg)
{
    auto& arg = cmd.add_flag_argument("flag")
	.set_tag("-f");
    
    std::vector<std::string> args = {"piet"};
    cmd.parse(args);
    EXPECT_FALSE(arg.get_value());

    args = {"piet", "-f"};
    cmd.parse(args);
    EXPECT_TRUE(arg.get_value());	
}

TEST_F(Pax, ThrowsOnSettingDefaultForRequiredArg)
{
    constexpr auto default_float = 1.f; 
    auto& arg = cmd.add_value_argument<float>("some float")
	.set_required(true);

    EXPECT_THROW(arg.set_default(default_float), std::logic_error);   
}

TEST_F(Pax, ThrowsOnSettingRequiredForArgWithDefault)
{
    constexpr auto default_float = 1.f; 
    auto& arg = cmd.add_value_argument<float>("some float")
	.set_default(default_float);

    EXPECT_THROW(arg.set_required(true), std::logic_error);   
}

TEST_F(Pax, DefaultValueIsPropagatedToBoundVariable)
{
    constexpr auto default_int = 1; 
    int q = 0;

    auto& arg1 = cmd.add_value_argument<int>("some int")
	.set_tag("-i")
	.set_long_tag("--int")
	.set_default(default_int)
	.bind(&q);

    EXPECT_EQ(q, default_int);

    // reverse bind/default order and test again
    auto& arg2 = cmd.add_value_argument<int>("some int")
	.set_tag("-i")
	.set_long_tag("--int")
	.bind(&q)
	.set_default(default_int + 1);

    EXPECT_EQ(q, default_int + 1);
}

TEST_F(Pax, RequiredArgWithoutValueIsInvalid)
{
    auto& arg = cmd.add_value_argument<int>("some integer")
	.set_tag("-i")
	.set_required(true);

    constexpr auto i = 5;
    std::vector<std::string> args = {"piet"};
    cmd.parse(args);
    EXPECT_FALSE(arg.is_valid());

    args = {"piet", "-i", std::to_string(i)};
    cmd.parse(args);
    EXPECT_TRUE(arg.is_valid());    
}

TEST_F(Pax, UnRequiredArgWithoutValueIsValid)
{
    auto& arg = cmd.add_value_argument<int>("some integer")
	.set_tag("-i");
    
    constexpr auto i = 5;
    std::vector<std::string> args = {"piet"};
    cmd.parse(args);
    EXPECT_TRUE(arg.is_valid());

    args = {"piet", "-i", std::to_string(i)};
    cmd.parse(args);
    EXPECT_TRUE(arg.is_valid());    
}

TEST_F(Pax, CanValidateValueArgument)
{
    auto validator = [](auto t) { return t > 3; };
    auto& arg = cmd.add_value_argument<int>("some integer")
	.set_tag("-i")
	.set_validator(validator);
    
    constexpr auto i = 5;
    std::vector<std::string> args = {"piet", "-i", std::to_string(i)};
    cmd.parse(args);
    EXPECT_TRUE(arg.is_valid());

    arg.set_validator([](auto t) { return t < 3; });
    EXPECT_FALSE(arg.is_valid());
}

TEST_F(Pax, ValidatesDefaultValueWhenNoValueGiven)
{
    auto validator = [](auto t) { return t > 3; };
    constexpr auto default_value = 4;
    auto& arg = cmd.add_value_argument<int>("some integer")
	.set_tag("-i")
	.set_default(default_value)
	.set_validator(validator);
    
    constexpr auto i = 5;
    std::vector<std::string> args = {"piet", "-i", std::to_string(i)};
    EXPECT_EQ(default_value, arg.get_value());
    EXPECT_TRUE(arg.is_valid());

    arg.set_validator([](auto t) { return t < 3; });
    EXPECT_FALSE(arg.is_valid());
}

TEST_F(Pax, CanParseAndValidatePath)
{
    auto& arg = cmd.add_value_argument<std::filesystem::path>("pth")
	.set_tag("-p")
	.set_validator([](auto pth) { return std::filesystem::exists(pth); });
    std::vector<std::string> args = {"piet", "-p", std::filesystem::current_path().string() };
    cmd.parse(args);
    EXPECT_TRUE(arg.is_valid());

    args.back() += "_not_exist....";
    cmd.parse(args);
    EXPECT_FALSE(arg.is_valid());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
