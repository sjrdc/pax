/*
 * this file is part of px - a command line argument parser in modern C++
 * Copyright (C) 2020 Sjoerd Crijns
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "px.h"

#include <gtest/gtest.h>
#include <filesystem>

namespace
{
    const auto all_less_than_3 = [](const auto& v)
    {
        return std::all_of(std::cbegin(v), std::cend(v), [](auto i) { return i < 3; });
    };
    const auto all_less_than_5 = [](const auto& v)
    {
        return std::all_of(std::cbegin(v), std::cend(v), [](auto i) { return i < 5; });
    };
}

class Px : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    px::command_line cli{ "cli" };
};

TEST_F(Px, CanParseIntegralValueArg)
{
    auto& arg = cli.add_value_argument<int>("some integer", "-i")
        .set_alternate_tag("--integer");

    constexpr auto i = 5;
    std::vector<std::string> args = { "piet", "-i", std::to_string(i) };
    cli.parse(args);

    EXPECT_EQ(5, arg.get_value());
}

TEST_F(Px, CanStoreIntegralValueInBoundVariable)
{
    int q;
    auto& arg = cli.add_value_argument<int>("some integer", "-i")
        .set_alternate_tag("--integer")
        .bind(&q);

    constexpr auto i = 5;
    const std::vector<std::string> args = { "piet", "-i", std::to_string(i) };
    cli.parse(args);

    EXPECT_EQ(q, i);
}

TEST_F(Px, CanParseFloatingPointValueArg)
{
    auto& arg = cli.add_value_argument<float>("some float", "-f")
        .set_alternate_tag("--float");

    constexpr auto f = 1.23f;
    const std::vector<std::string> args = { "piet", "-f", std::to_string(f) };
    cli.parse(args);

    EXPECT_FLOAT_EQ(f, arg.get_value());
}

TEST_F(Px, FlagArgValueIsFalseByDefault)
{
    auto& arg = cli.add_flag_argument("flag", "-f");
    EXPECT_FALSE(arg.get_value());
}

TEST_F(Px, CanParseFlagArg)
{
    auto& arg = cli.add_flag_argument("flag", "-f");

    std::vector<std::string> args = { "piet" };
    cli.parse(args);
    EXPECT_FALSE(arg.get_value());

    args = { "piet", "-f" };
    cli.parse(args);
    EXPECT_TRUE(arg.get_value());
}

TEST_F(Px, RequiredArgWithoutValueIsInvalid)
{
    auto& arg = cli.add_value_argument<int>("some integer", "-i")
        .set_required(true);

    constexpr auto i = 5;
    std::vector<std::string> args = { "piet" };
    cli.parse(args);
    EXPECT_FALSE(arg.is_valid());

    args = { "piet", "-i", std::to_string(i) };
    cli.parse(args);
    EXPECT_TRUE(arg.is_valid());
}

TEST_F(Px, UnRequiredArgWithoutValueIsValid)
{
    auto& arg = cli.add_value_argument<int>("some integer", "-i");

    constexpr auto i = 5;
    std::vector<std::string> args = { "piet" };
    cli.parse(args);
    EXPECT_TRUE(arg.is_valid());

    args = { "piet", "-i", std::to_string(i) };
    cli.parse(args);
    EXPECT_TRUE(arg.is_valid());
}

TEST_F(Px, CanValidateValueArgument)
{
    auto validator = [](auto t) { return t > 3; };
    auto& arg = cli.add_value_argument<int>("some integer", "-i")
        .set_validator(validator);

    constexpr auto i = 5;
    const std::vector<std::string> args = { "piet", "-i", std::to_string(i) };
    cli.parse(args);
    EXPECT_TRUE(arg.is_valid());

    arg.set_validator([](auto t) { return t < 3; });
    EXPECT_FALSE(arg.is_valid());
}

TEST_F(Px, CanParseAndValidatePath)
{
    auto& arg = cli.add_value_argument<std::filesystem::path>("pth", "-p")
        .set_validator([](auto pth) { return std::filesystem::exists(pth); });
    std::vector<std::string> args = { "piet", "-p", std::filesystem::current_path().string() };
    cli.parse(args);
    EXPECT_TRUE(arg.is_valid());

    args.back() += "_not_exist....";
    cli.parse(args);
    EXPECT_FALSE(arg.is_valid());
}

TEST_F(Px, CanParseMultiArg)
{
    auto& arg = cli.add_multi_value_argument<int>("multiple integers", "--ints");

    const std::vector<int> v = { 1, 2, 3, 4 };
    std::vector<std::string> args = { "piet", "--ints" };
    for (const auto i : { 1, 2, 3, 4 })
    {
        args.push_back(std::to_string(i));
    }

    cli.parse(args);

    ASSERT_EQ(v.size(), arg.get_value().size());
    EXPECT_TRUE(std::equal(std::begin(v), std::end(v), std::cbegin(arg.get_value())));
}

TEST_F(Px, RequiredMultiArgWithoutValueIsInvalid)
{
    auto& arg = cli.add_multi_value_argument<int>("multiple integers", "--ints")
        .set_required(true);

    EXPECT_FALSE(arg.is_valid());

    const std::vector<int> v = { 1, 2, 3, 4 };
    std::vector<std::string> args = { "piet", "--ints" };
    for (const auto i : { 1, 2, 3, 4 })
    {
        args.push_back(std::to_string(i));
    }
    cli.parse(args);
    EXPECT_TRUE(arg.is_valid());
}

TEST_F(Px, CanValidateMultiArgWithCustomValidator)
{
    auto& arg = cli.add_multi_value_argument<int>("multiple integers", "--ints")
        .set_required(true)
        .set_validator(all_less_than_3);

    const std::vector<int> v = { 1, 2, 3, 4 };
    std::vector<std::string> args = { "piet", "--ints" };
    for (const auto i : { 1, 2, 3, 4 })
    {
        args.push_back(std::to_string(i));
    }
    cli.parse(args);
    EXPECT_FALSE(arg.is_valid());

    arg.set_validator(all_less_than_5);
    EXPECT_TRUE(arg.is_valid());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
