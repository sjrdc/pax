/*
  px - a command line argument parser in modern C++
  Copyright (C) 2020 Sjoerd Crijns

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "px.h"

#include <gtest/gtest.h>
#include <filesystem>

class Px : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    px::command_line cmd {"cmd"};    
};

TEST_F(Px, CanParseIntegralValueArg)
{
    auto& arg = cmd.add_value_argument<int>("some integer", "-i")
	.set_alternate_tag("--integer");

    constexpr auto i = 5;
    std::vector<std::string> args = {"piet", "-i", std::to_string(i)};
    cmd.parse(args);

    EXPECT_EQ(5, arg.get_value());
}

TEST_F(Px, CanStoreIntegralValueInBoundVariable)
{
    int q;
    auto& arg = cmd.add_value_argument<int>("some integer", "-i")
	.set_alternate_tag("--integer")
	.bind(&q);

    constexpr auto i = 5;
    std::vector<std::string> args = {"piet", "-i", std::to_string(i)};
    cmd.parse(args);

    EXPECT_EQ(q, i);
}

TEST_F(Px, CanParseFloatingPointValueArg)
{
    auto& arg = cmd.add_value_argument<float>("some float", "-f")
	.set_alternate_tag("--float");

    constexpr auto f = 1.23f;
    std::vector<std::string> args = {"piet", "-f", std::to_string(f)};
    cmd.parse(args);

    EXPECT_FLOAT_EQ(f, arg.get_value());
}

TEST_F(Px, FlagArgValueIsFalseByDefault)
{
    auto& arg = cmd.add_flag_argument("flag", "-f");
    EXPECT_FALSE(arg.get_value());
}

TEST_F(Px, CanParseFlagArg)
{
    auto& arg = cmd.add_flag_argument("flag", "-f");
    
    std::vector<std::string> args = {"piet"};
    cmd.parse(args);
    EXPECT_FALSE(arg.get_value());

    args = {"piet", "-f"};
    cmd.parse(args);
    EXPECT_TRUE(arg.get_value());	
}

TEST_F(Px, RequiredArgWithoutValueIsInvalid)
{
    auto& arg = cmd.add_value_argument<int>("some integer", "-i")
	.set_required(true);

    constexpr auto i = 5;
    std::vector<std::string> args = {"piet"};
    cmd.parse(args);
    EXPECT_FALSE(arg.is_valid());

    args = {"piet", "-i", std::to_string(i)};
    cmd.parse(args);
    EXPECT_TRUE(arg.is_valid());    
}

TEST_F(Px, UnRequiredArgWithoutValueIsValid)
{
    auto& arg = cmd.add_value_argument<int>("some integer", "-i");
    
    constexpr auto i = 5;
    std::vector<std::string> args = {"piet"};
    cmd.parse(args);
    EXPECT_TRUE(arg.is_valid());

    args = {"piet", "-i", std::to_string(i)};
    cmd.parse(args);
    EXPECT_TRUE(arg.is_valid());    
}

TEST_F(Px, CanValidateValueArgument)
{
    auto validator = [](auto t) { return t > 3; };
    auto& arg = cmd.add_value_argument<int>("some integer", "-i")
	.set_validator(validator);
    
    constexpr auto i = 5;
    std::vector<std::string> args = {"piet", "-i", std::to_string(i)};
    cmd.parse(args);
    EXPECT_TRUE(arg.is_valid());

    arg.set_validator([](auto t) { return t < 3; });
    EXPECT_FALSE(arg.is_valid());
}

TEST_F(Px, CanParseAndValidatePath)
{
    auto& arg = cmd.add_value_argument<std::filesystem::path>("pth", "-p")
	.set_validator([](auto pth) { return std::filesystem::exists(pth); });
    std::vector<std::string> args = {"piet", "-p", std::filesystem::current_path().string() };
    cmd.parse(args);
    EXPECT_TRUE(arg.is_valid());

    args.back() += "_not_exist....";
    cmd.parse(args);
    EXPECT_FALSE(arg.is_valid());
}

TEST_F(Px ,CanParseMultiArg)
{
    auto& arg = cmd.add_multi_value_argument<int>("multiple integers", "--ints");

    const std::vector<int> v = {1, 2, 3, 4};
    std::vector<std::string> args = {"piet", "--ints"};
    for (const auto i : {1, 2, 3, 4})
    {
	args.push_back(std::to_string(i));
    }

    ASSERT_EQ(v.size(), arg.get_value().size());
    EXPECT_TRUE(std::equal(std::begin(v), std::end(v), std::cbegin(arg.get_value())));
}

#include <filesystem>

namespace fs = std::filesystem;

void show_kittens(int i)
{
}

void store_kittens(const fs::path& p)
{
}

int piet(int argc, char** argv)
{
  int i = 1;
  fs::path pth;
  
  px::command_line cli("the program name");
  cli.add_value_argument<int>("integer", "-i")
    .set_description("the number of kittens to show")
    .set_validator([](auto i) { return i > 0 && i <= 5; })
    .bind(&i);
  cli.add_value_argument<fs::path>("path", "-p")
    .set_required(true)
    .set_description("the path to use for storage of the shown kittens")
    .bind(&pth)
    .set_validator([](auto p) { return fs::exists(p) && fs::is_regular_file(p); });
  auto& help_arg = cli.add_flag_argument("help", "-h")
    .set_alternate_tag("--help")
      .set_description("show this help message");
  
  cli.parse(argc, argv);
  if (help_arg.get_value())
  {
    cli.print_help(std::cout);
    return 0;
  }
  
  show_kittens(i);
  store_kittens(pth);
  
  return 0;
}

int main(int argc, char** argv)
{
    piet(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
