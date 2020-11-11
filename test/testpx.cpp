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

namespace px_tests
{
    class px_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
        }

        px::command_line cli{ "cli" };
    };

    TEST_F(px_test, throws_on_adding_tag_arg_after_positional_arg)
    {
        cli.add_positional_argument<int>("integer");
        EXPECT_THROW(cli.add_flag_argument("flag", "-f"), std::logic_error);
    }

    TEST_F(px_test, can_parse_positional_arg_after_tag_args)
    {
        auto flag = false;
        auto q = 0;
        auto r = 0;
        cli.add_flag_argument("flag", "-f")
            .bind(&flag);
        cli.add_value_argument<int>("integer", "-i")
            .bind(&q);
        cli.add_positional_argument<int>("integer")
            .bind(&r);

        constexpr auto i = 4;
        constexpr auto j = 3;
        const std::vector<std::string> args{ "piet", "-i", std::to_string(i), "-f", "--", std::to_string(j) };
        cli.parse(args);

        EXPECT_EQ(q, i);
        EXPECT_EQ(j, r);
        EXPECT_TRUE(flag);
    }

    TEST_F(px_test, can_parse_positional_arg_when_tag_args_specified)
    {
        auto flag = false;
        auto q = 0;
        auto r = 0;
        cli.add_flag_argument("flag", "-f")
            .bind(&flag);
        cli.add_value_argument<int>("integer", "-i")
            .bind(&q);
        cli.add_positional_argument<int>("integer")
            .bind(&r);

        constexpr auto j = 3;
        std::vector<std::string> args{ "piet", std::to_string(j) };
        cli.parse(args);

        EXPECT_EQ(j, r);
        EXPECT_FALSE(flag);

        // the same thing but with a separator
        args = { "piet", "--", std::to_string(j) };
        cli.parse(args);

        EXPECT_EQ(j, r);
        EXPECT_FALSE(flag);
    }

    class px_positional_arg_test : public px_test
    {
    protected:
        void SetUp() override
        {
        }
    };

    TEST_F(px_positional_arg_test, throws_on_missing_positional_arg)
    {
        auto q = 0;
        cli.add_positional_argument<int>("integer")
            .bind(&q);

        const std::vector<std::string> args = { "piet" };
        EXPECT_THROW(cli.parse(args), std::runtime_error);
    }

    TEST_F(px_positional_arg_test, can_parse_integral_arg)
    {
        auto& arg = cli.add_positional_argument<int>("integer");

        constexpr auto i = 5;
        const std::vector<std::string> args = { "piet", std::to_string(i) };

        EXPECT_NO_THROW(cli.parse(args));
        EXPECT_EQ(i, arg.get_value());
    }

    TEST_F(px_positional_arg_test, can_parse_string_arg)
    {
        auto& arg = cli.add_positional_argument<std::string>("string");

        static const std::string s("piet");
        const std::vector<std::string> args = { "piet", s };

        EXPECT_NO_THROW(cli.parse(args));
        EXPECT_EQ(s, arg.get_value());
    }

    TEST_F(px_positional_arg_test, can_store_integral_value_in_bound_variable)
    {
        auto q = 0;
        auto& arg = cli.add_positional_argument<int>("integer")
            .bind(&q);

        constexpr auto i = 5;
        const std::vector<std::string> args = { "piet", std::to_string(i) };

        EXPECT_NO_THROW(cli.parse(args));
        EXPECT_EQ(i, q);
        EXPECT_EQ(i, arg.get_value());
    }

    class px_value_arg_test : public px_test
    {
    protected:
        void SetUp() override
        {
        }
    };

    TEST_F(px_value_arg_test, can_parse_integral_value_arg)
    {
        auto& arg = cli.add_value_argument<int>("some integer", "-i")
            .set_alternate_tag("--integer");

        constexpr auto i = 5;
        const std::vector<std::string> args = { "piet", "-i", std::to_string(i) };
        cli.parse(args);

        EXPECT_EQ(i, arg.get_value());
    }

    TEST_F(px_value_arg_test, can_store_integral_value_in_bound_variable)
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

    TEST_F(px_value_arg_test, can_parse_float_value_arg)
    {
        auto& arg = cli.add_value_argument<float>("some float", "-f")
            .set_alternate_tag("--float");

        constexpr auto f = 1.23f;
        const std::vector<std::string> args = { "piet", "-f", std::to_string(f) };
        cli.parse(args);

        EXPECT_FLOAT_EQ(f, arg.get_value());
    }

    TEST_F(px_value_arg_test, throws_on_getting_value_from_valueless_arg)
    {
        auto& arg = cli.add_value_argument<float>("some float", "-f")
            .set_required(true);
        // required and no value -> invalid
        EXPECT_THROW(arg.get_value(), std::runtime_error);
        EXPECT_FALSE(arg.is_valid());
    }

    TEST_F(px_value_arg_test, can_parse_string_value_arg)
    {
        auto& arg = cli.add_value_argument<std::string>("some string", "-s")
            .set_alternate_tag("--string");

        const std::string s("jannssen");
        const std::vector<std::string> args = { "piet", "-s", s };
        cli.parse(args);

        EXPECT_EQ(s, arg.get_value());
    }

    TEST_F(px_value_arg_test, RequiredArgWithoutValueIsInvalid)
    {
        auto& arg = cli.add_value_argument<int>("some integer", "-i")
            .set_required(true);

        constexpr auto i = 5;
        std::vector<std::string> args = { "piet" };
        EXPECT_THROW(cli.parse(args), std::runtime_error);
        EXPECT_FALSE(arg.is_valid());

        args = { "piet", "-i", std::to_string(i) };
        cli.parse(args);
        EXPECT_TRUE(arg.is_valid());
    }

    TEST_F(px_value_arg_test, unrequired_arg_without_value_is_valid)
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

    TEST_F(px_value_arg_test, can_validate_value_argument)
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

    TEST_F(px_value_arg_test, can_parse_and_validate_path)
    {
        auto& arg = cli.add_value_argument<std::filesystem::path>("pth", "-p")
            .set_validator([](auto pth) { return std::filesystem::exists(pth); });
        std::vector<std::string> args = { "piet", "-p", std::filesystem::current_path().string() };
        cli.parse(args);
        EXPECT_TRUE(arg.is_valid());

        args.back() += "_not_exist....";
        EXPECT_THROW(cli.parse(args), std::runtime_error);
    }

    class px_flag_arg_test : public px_test
    {
    protected:
        void SetUp() override
        {
            flag_arg = std::ref(cli.add_flag_argument("flag", "-f"));
        }

        std::optional<std::reference_wrapper<px::tag_argument<bool, px::scalar<bool>>>> flag_arg;
    };

    TEST_F(px_flag_arg_test, flag_arg_value_is_false_by_default)
    {
        EXPECT_FALSE(flag_arg->get().get_value());
    }

    TEST_F(px_flag_arg_test, can_parse_flag_arg)
    {
        std::vector<std::string> args = { "piet" };
        cli.parse(args);
        EXPECT_FALSE(flag_arg->get().get_value());

        args = { "piet", "-f" };
        cli.parse(args);
        EXPECT_TRUE(flag_arg->get().get_value());
    }

    TEST_F(px_flag_arg_test, can_store_value_in_bound_variable)
    {
        bool q = false;
        std::vector<std::string> args = { "piet" };
        flag_arg->get().bind(&q);
        cli.parse(args);
        EXPECT_FALSE(flag_arg->get().get_value());
        EXPECT_FALSE(q);

        args = { "piet", "-f" };
        cli.parse(args);
        EXPECT_TRUE(flag_arg->get().get_value());
        EXPECT_TRUE(q);
    }

    class px_multi_value_arg_test : public px_value_arg_test
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

    TEST_F(px_multi_value_arg_test, can_parse_multi_integral_arg)
    {
        auto& arg = cli.add_multi_value_argument<int>("multiple integers", "--ints");

        cli.parse(make_multi_arg());

        ASSERT_EQ(v.size(), arg.get_value().size());
        EXPECT_TRUE(std::equal(std::begin(v), std::end(v), std::cbegin(arg.get_value())));
    }

    TEST_F(px_multi_value_arg_test, can_parse_multi_string_arg)
    {
        auto& arg = cli.add_multi_value_argument<std::string>("multiple strings", "--strings");

        cli.parse({ "piet", "--strings", "s0", "s1", "s2", "s3" });
        EXPECT_EQ(4, arg.get_value().size());
    }

    TEST_F(px_multi_value_arg_test, required_multi_arg_without_value_is_invalid)
    {
        auto& arg = cli.add_multi_value_argument<int>("multiple integers", "--ints")
            .set_required(true);

        EXPECT_FALSE(arg.is_valid());

        EXPECT_NO_THROW(cli.parse(make_multi_arg()));
        EXPECT_TRUE(arg.is_valid());
    }

    TEST_F(px_multi_value_arg_test, throws_on_getting_value_from_invalid_multi_arg)
    {
        auto& arg = cli.add_multi_value_argument<int>("multiple integers", "--ints")
            .set_required(true);
        // required + does not have a value -> invalid
        EXPECT_THROW(arg.get_value(), std::runtime_error);
    }

    TEST_F(px_multi_value_arg_test, can_store_integral_value_in_bound_variable)
    {
        std::vector<int> q;
        auto& arg = cli.add_multi_value_argument<int>("some integer", "--ints")
            .bind(&q);

        cli.parse(make_multi_arg());

        EXPECT_EQ(q, v);
    }

    TEST_F(px_multi_value_arg_test, can_validate_multi_arg_with_custom_validator)
    {
        auto& arg = cli.add_multi_value_argument<int>("multiple integers", "--ints")
            .set_required(true)
            .set_validator(all_less_than_3);

        EXPECT_THROW(cli.parse(make_multi_arg()), std::runtime_error);
        EXPECT_FALSE(arg.is_valid());

        arg.set_validator(all_less_than_5);
        EXPECT_TRUE(arg.is_valid());
    }
}
