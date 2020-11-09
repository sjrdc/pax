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

class PxTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    px::command_line cli{ "cli" };
};

class PxValueArgTest : public PxTest
{
protected:
    void SetUp() override
    {
    }    
};

TEST_F(PxValueArgTest, CanParseIntegralValueArg)
{
    auto& arg = cli.add_value_argument<int>("some integer", "-i")
        .set_alternate_tag("--integer");

    constexpr auto i = 5;
    std::vector<std::string> args = { "piet", "-i", std::to_string(i) };
    cli.parse(args);

    EXPECT_EQ(i, arg.get_value());
}

TEST_F(PxValueArgTest, CanStoreIntegralValueInBoundVariable)
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

TEST_F(PxValueArgTest, CanParseFloatingPointValueArg)
{
    auto& arg = cli.add_value_argument<float>("some float", "-f")
        .set_alternate_tag("--float");

    constexpr auto f = 1.23f;
    const std::vector<std::string> args = { "piet", "-f", std::to_string(f) };
    cli.parse(args);

    EXPECT_FLOAT_EQ(f, arg.get_value());
}

TEST_F(PxValueArgTest, ThrowsOnGettingValueFromValuelessArg)
{
    auto& arg = cli.add_value_argument<float>("some float", "-f")
        .set_required("--float");
    // required and no value -> invalid
    EXPECT_THROW(arg.get_value(), std::runtime_error);
}

TEST_F(PxValueArgTest, CanParseStringValueArg)
{
    auto& arg = cli.add_value_argument<std::string>("some string", "-s")
        .set_alternate_tag("--string");

    const std::string s("jannssen");
    const std::vector<std::string> args = { "piet", "-s", s };
    cli.parse(args);

    EXPECT_EQ(s, arg.get_value());
}

TEST_F(PxValueArgTest, RequiredArgWithoutValueIsInvalid)
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

TEST_F(PxValueArgTest, UnRequiredArgWithoutValueIsValid)
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

TEST_F(PxValueArgTest, CanValidateValueArgument)
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

TEST_F(PxValueArgTest, CanParseAndValidatePath)
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


class PxFlagArgTest : public PxTest
{
protected:
    void SetUp() override
    {
    }
};

TEST_F(PxFlagArgTest, FlagArgValueIsFalseByDefault)
{
    auto& arg = cli.add_flag_argument("flag", "-f");
    EXPECT_FALSE(arg.get_value());
}

TEST_F(PxFlagArgTest, CanParseFlagArg)
{
    auto& arg = cli.add_flag_argument("flag", "-f");

    std::vector<std::string> args = { "piet" };
    cli.parse(args);
    EXPECT_FALSE(arg.get_value());

    args = { "piet", "-f" };
    cli.parse(args);
    EXPECT_TRUE(arg.get_value());
}


class PxMultiValueArgTest : public PxValueArgTest
{
protected:
    void SetUp() override
    {

    }

    auto make_multi_arg() const
    {
        std::vector<std::string> args = { "piet", "--ints" };
        for (const auto& i : v)
        {
            args.push_back(std::to_string(i));
        }
        return args;
    }
    const std::vector<int> v = { 1, 2, 3, 4 };
};

TEST_F(PxMultiValueArgTest, CanParseMultiIntegralArg)
{
    auto& arg = cli.add_multi_value_argument<int>("multiple integers", "--ints");

    cli.parse(make_multi_arg());

    ASSERT_EQ(v.size(), arg.get_value().size());
    EXPECT_TRUE(std::equal(std::begin(v), std::end(v), std::cbegin(arg.get_value())));
}

TEST_F(PxMultiValueArgTest, CanParseMultiStringArg)
{
    auto& arg = cli.add_multi_value_argument<std::string>("multiple strings", "--strings");

    cli.parse({ "piet", "--strings", "s0", "s1", "s2", "s3" });
    EXPECT_EQ(4, arg.get_value().size());
}

TEST_F(PxMultiValueArgTest, RequiredMultiArgWithoutValueIsInvalid)
{
    auto& arg = cli.add_multi_value_argument<int>("multiple integers", "--ints")
        .set_required(true);

    EXPECT_FALSE(arg.is_valid());

    cli.parse(make_multi_arg());
    EXPECT_TRUE(arg.is_valid());
}

TEST_F(PxMultiValueArgTest, CanValidateMultiArgWithCustomValidator)
{
    auto& arg = cli.add_multi_value_argument<int>("multiple integers", "--ints")
        .set_required(true)
        .set_validator(all_less_than_3);

    cli.parse(make_multi_arg());
    EXPECT_FALSE(arg.is_valid());

    arg.set_validator(all_less_than_5);
    EXPECT_TRUE(arg.is_valid());
}

TEST_F(PxMultiValueArgTest, ThrowsOnGettingValueFromInvalidMultiArg)
{
    auto& arg = cli.add_multi_value_argument<int>("multiple integers", "--ints")
        .set_required(true);
    // required + does not have a value -> invalid
    EXPECT_THROW(arg.get_value(), std::runtime_error);
}
