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

module;

#include <algorithm>
#include <cctype>
#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <span>
#include <string_view>
#include <vector>
#include <iterator>
#include <concepts>

export module px;

namespace detail
{
    auto pad_right(std::string_view s, decltype(s.size()) n)
    {
        constexpr decltype(s.size()) zero {0};
        return std::string(s).append(std::max(zero, n - s.size()), ' ');
    }

    template <typename T>
    auto parse_scalar(const std::string& s)
    {
        if constexpr (std::is_same_v<std::string, T>)
        {
            return s;
        }
        else
        {
            std::istringstream stream(s);
            T t;
            stream >> t;
            if (!stream.eof() || stream.fail())
            {
                throw std::runtime_error("could not parse from '" + s + "'");
            }
            return t;
        }
    }

    auto is_separator_tag(std::string_view s)
    {
        return s.size() == 2 && s[0] == '-' && s[1] == '-';
    }

    auto is_short_tag(std::string_view s)
    {
        return (s.size() > 1 && s[0] == '-' && !std::isdigit(s[1]));
    }

    auto is_alternate_tag(std::string_view s)
    {
        return (s.size() > 2 && s[0] == '-' && s[1] == '-' && !std::isdigit(s[2]));
    }

    auto is_tag(const std::string& s)
    {
        return !s.empty() && !is_separator_tag(s) &&
             (is_short_tag(s) || is_alternate_tag(s));
    }

    auto find_invalid(const auto& begin, const auto& end)
    {
        return std::find_if_not(begin, end, [](auto& arg) { return arg->is_valid(); });
    }

    void throw_on_invalid(const auto& begin, const auto& end)
    {
        if (auto invalid_arg = find_invalid(begin, end); invalid_arg != end)
        {
            throw std::runtime_error("argument '" + (*invalid_arg)->get_name() + "' invalid after parsing");
        }
    }
}

export namespace px
{
    template <typename T>
    class scalar
    {
    public:
        using value_type = T;
        bool has_value() const;
        const value_type& get_value() const;
        template <std::input_iterator iterator>
        iterator parse(const iterator& begin, const iterator& end);

    private:
        std::optional<value_type> value = std::nullopt;
    };

    template <>
    class scalar<bool>
    {
    public:
        using value_type = bool;
        bool has_value() const;
        const value_type& get_value() const;
        template <std::input_iterator iterator>
        iterator parse(const iterator& begin, const iterator& end);
    private:
        bool value = false;
    };

    template <typename T>
    class multi_scalar
    {
    public:
        using value_type = std::vector<T>;
        bool has_value() const;
        const value_type& get_value() const;

        template <std::input_iterator iterator>
        iterator parse(const iterator& begin, const iterator& end);

    private:
        value_type value;
    };

    using argv_iterator = std::span<const std::string>::iterator;
    class iargument
    {
    public:
        virtual ~iargument() = default;
        virtual void print_help(std::ostream&) const = 0;
        virtual argv_iterator parse(const argv_iterator&, const argv_iterator&) = 0;
        virtual bool is_valid() const = 0;

        virtual const std::string& get_name() const = 0;
        virtual const std::string& get_description() const = 0;
    };

    template <typename Derived>
    class argument : public iargument
    {
    public:
        argument(std::string_view n);
        virtual ~argument() = default;

        const std::string& get_name() const override;
        const std::string& get_description() const override;
        Derived& set_description(std::string_view d);

    private:
        Derived* this_as_derived() { return reinterpret_cast<Derived*>(this); }
        std::string name;
        std::string description;
    };

    template <typename T>
    class positional_argument : public argument<positional_argument<T>>
    {
    public:
        using value_type = T;
        using validation_function = std::function<bool(const value_type&)>;
        using base = argument<positional_argument<T>>;

        positional_argument(std::string_view n);
        virtual ~positional_argument() = default;

        void print_help(std::ostream&) const override;
        argv_iterator parse(const argv_iterator&, const argv_iterator&) override;
        bool is_valid() const override;

        const value_type& get_value() const;
        positional_argument<T>& bind(T*);

        positional_argument<T>& set_validator(validation_function);

    private:
        std::optional<value_type> value = std::nullopt;
        value_type* bound_variable = nullptr;
        validation_function validator = [](const auto&) { return true; };
    };

    template <typename T, typename storage = scalar<T>>
    class tag_argument : public argument<tag_argument<T, storage>>
    {
    public:
        using value_type = typename storage::value_type;
        using validation_function = std::function<bool(const value_type&)>;
        using base = argument<tag_argument<T, storage>>;

        tag_argument(std::string_view n, std::string_view t);
        virtual ~tag_argument() = default;

        const value_type& get_value() const;
        tag_argument<T, storage>& bind(value_type*);

        template <typename U = T, typename = std::enable_if<!std::is_same_v<U, bool>>>
        bool is_required() const;
        template <typename U = T, typename = std::enable_if<!std::is_same_v<U, bool>>>
        tag_argument<T, storage>& set_required(bool);

        template <typename U = T, typename = std::enable_if<!std::is_same_v<U, bool>>>
        tag_argument<T, storage>& set_validator(validation_function f);
        bool is_valid() const override;

        void print_help(std::ostream&) const override;
        argv_iterator parse(const argv_iterator&, const argv_iterator&) override;

        const std::string& get_tag() const;
        const std::string& get_alternate_tag() const;
        tag_argument<T, storage>& set_alternate_tag(std::string_view);

    private:
        bool matches(std::string_view) const;
        std::string tag;
        std::string alternate_tag;
        storage value;
        value_type* bound_variable = nullptr;
        bool required = false;
        validation_function validator = [](const auto&) { return true; };
    };

    class command_line
    {
    public:
        command_line(std::string_view program_name);

        tag_argument<bool, scalar<bool>>& add_flag_argument(std::string_view, std::string_view);
        template <typename T>
        tag_argument<T>& add_value_argument(std::string_view, std::string_view);
        template <typename T>
        tag_argument<T, multi_scalar<T>>& add_multi_value_argument(std::string_view name, std::string_view);
        template <typename T>
        positional_argument<T>& add_positional_argument(std::string_view);

        void print_help(std::ostream&);
        void parse(std::span<const std::string>);
        void parse(int argc, char** argv);

    private:
        void prevent_tag_args_after_positional_args();

        std::string name;
        std::string description;
        std::vector<std::unique_ptr<iargument>> arguments;
        std::vector<std::unique_ptr<iargument>> positional_arguments;
    };

    template <typename T>
    const typename scalar<T>::value_type& scalar<T>::get_value() const
    {
        if (has_value())
        {
            return *value;
        }
        else
        {
            throw std::runtime_error("does not have value");
        }
    }

    template <typename T>
    bool scalar<T>::has_value() const
    {
        return value.has_value();
    }

    template <typename T>
    template <std::input_iterator iterator>
    iterator scalar<T>::parse(const iterator& begin, const iterator& end)
    {
        value = detail::parse_scalar<T>(*begin);
        return begin;
    }

    inline const scalar<bool>::value_type& scalar<bool>::get_value() const
    {
        return value;
    }

    inline bool scalar<bool>::has_value() const
    {
        return true;
    }

    template <std::input_iterator iterator>
    iterator scalar<bool>::parse(const iterator& begin, const iterator& end)
    {
        value = true;
        return begin;
    }

    template <typename T>
    bool multi_scalar<T>::has_value() const
    {
        return !std::empty(value);
    }

    template <typename T>
    const typename multi_scalar<T>::value_type& multi_scalar<T>::get_value() const
    {
        return value;
    }

    template <typename T>
    template <std::input_iterator iterator>
    iterator multi_scalar<T>::parse(const iterator& begin, const iterator& end)
    {
        auto i = begin;
        if (std::distance(begin, end) > 0)
        {
            for (; i != end && !detail::is_tag(*i); ++i)
            {
                value.push_back(detail::parse_scalar<T>(*i));
            }
            --i;
        }
        return i;
    }

    template <typename T>
    inline argument<T>::argument(std::string_view n) :
        name(n)
    {
    }

    template <typename T>
    inline const std::string& argument<T>::get_name() const
    {
        return name;
    }

    template <typename T>
    inline T& argument<T>::set_description(std::string_view d)
    {
        description = d;
        return *this_as_derived();
    }

    template <typename T>
    inline const std::string& argument<T>::get_description() const
    {
        return description;
    }

    template <typename T>
    positional_argument<T>::positional_argument(std::string_view n) :
    positional_argument<T>::base(n)
    {
    }
    
    template <typename T>
    positional_argument<T>& positional_argument<T>::bind(positional_argument<T>::value_type* t)
    {
        bound_variable = t;
        return *this;
    }

    template <typename T>
    void positional_argument<T>::print_help(std::ostream& o) const
    {
        o << "   "
            << base::get_name() << " "
            << base::get_description()
            << "\n";
    }

    template <typename T>
    positional_argument<T>& positional_argument<T>::set_validator(validation_function f)
    {
        validator = std::move(f);
    }

    template <typename T>
    bool positional_argument<T>::is_valid() const
    {
        return value.has_value() && validator(*value);
    }

    template <typename T>
    const typename positional_argument<T>::value_type& positional_argument<T>::get_value() const
    {
        if (value.has_value())
        {
            return *value;
        }
        else
        {
            throw std::runtime_error("getting value from invalid argument '" + base::get_name() + "'");
        }
    }

    template <typename T>
    argv_iterator
        positional_argument<T>::parse(const argv_iterator& begin,
            const argv_iterator& end)
    {
        value = detail::parse_scalar<value_type>(*begin);
        if (bound_variable != nullptr)
        {
            *bound_variable = *value;
        }
        return begin;
    }

    template <typename T, typename storage>
    bool tag_argument<T, storage>::matches(std::string_view s) const
    {
        return (!tag.empty() && tag == s) || (!alternate_tag.empty() && alternate_tag == s);
    }

    template <typename T, typename storage>
    const std::string& tag_argument<T, storage>::get_tag() const
    {
        return tag;
    }

    template <typename T, typename storage>
    const std::string& tag_argument<T, storage>::get_alternate_tag() const
    {
        return alternate_tag;
    }

    template <typename T, typename storage>
    tag_argument<T, storage>& tag_argument<T, storage>::set_alternate_tag(std::string_view t)
    {
        alternate_tag = t;
        return *this;
    }

    template <typename T, typename storage>
    tag_argument<T, storage>::tag_argument(std::string_view n, std::string_view t) :
        argument<tag_argument<T, storage>>(n),
        tag(t)
    {
    }

    template <typename T, typename storage>
    tag_argument<T, storage>& tag_argument<T, storage>::bind(value_type* t)
    {
        bound_variable = t;
        return *this;
    }

    template <typename T, typename storage>
    template <typename U, typename>
    bool tag_argument<T, storage>::is_required() const
    {
        return required;
    }

    template <typename T, typename storage>
    template <typename U, typename>
    tag_argument<T, storage>& tag_argument<T, storage>::set_required(bool r)
    {
        required = r;
        return *this;
    }

    template <typename T, typename storage>
    template <typename U, typename>
    tag_argument<T, storage>& tag_argument<T, storage>::set_validator(validation_function f)
    {
        validator = std::move(f);
        return *this;
    }

    template <typename T, typename storage>
    const typename tag_argument<T, storage>::value_type& tag_argument<T, storage>::get_value() const
    {
        if (!is_valid())
        {
            throw std::runtime_error("getting value from invalid argument '" + base::get_name() + "'");
        }
        return value.get_value();
    }

    template <typename T, typename storage>
    void tag_argument<T, storage>::print_help(std::ostream& o) const
    {
        constexpr auto alternate_tag_size = 15;
        o << "   "
            << tag
            << ((!alternate_tag.empty()) ?
                ", " + detail::pad_right(alternate_tag, alternate_tag_size - 2) :
                detail::pad_right("", alternate_tag_size))
            << ((required) ? "(required) " : "")
            << base::get_description()
            << "\n";
    }

    template <typename T, typename storage>
    bool tag_argument<T, storage>::is_valid() const
    {
        if (value.has_value())
        {
            return validator(value.get_value());
        }
        else return !required;
    }

    template <typename T, typename storage>
    argv_iterator
        tag_argument<T, storage>::parse(const argv_iterator& begin,
            const argv_iterator& end)
    {
        if (std::distance(begin, end) >= 1 && matches(*begin))
        {
            auto i = begin;
            if constexpr (!std::is_same_v<T, bool>) // i.e. flag arg
            {
                ++i;
            }
            i = value.parse(i, end);
            if (bound_variable != nullptr)
            {
                *bound_variable = value.get_value();
            }
            return i;
        }
        else
        {
            return begin;
        }
    }

    command_line::command_line(std::string_view program_name) :
        name(program_name)
    {
    }

    template <typename T>
    inline positional_argument<T>& command_line::add_positional_argument(std::string_view name)
    {
        auto arg = std::make_unique<positional_argument<T>>(name);
        auto& ref = *arg;
        positional_arguments.push_back(std::move(arg));
        return ref;
    }

    inline tag_argument<bool, scalar<bool>>& command_line::add_flag_argument(std::string_view name, std::string_view tag)
    {
        prevent_tag_args_after_positional_args();
        auto arg = std::make_unique<tag_argument<bool, scalar<bool>>>(name, tag);
        auto& ref = *arg;
        arguments.push_back(std::move(arg));
        return ref;
    }

    template <typename T>
    tag_argument<T>& command_line::add_value_argument(std::string_view name, std::string_view tag)
    {
        prevent_tag_args_after_positional_args();
        auto arg = std::make_unique<tag_argument<T>>(name, tag);
        auto& ref = *arg;
        arguments.push_back(std::move(arg));
        return ref;
    }

    template <typename T>
    tag_argument<T, multi_scalar<T>>& command_line::add_multi_value_argument(std::string_view name, std::string_view tag)
    {
        prevent_tag_args_after_positional_args();
        auto arg = std::make_unique<tag_argument<T, multi_scalar<T>>>(name, tag);
        auto& ref = *arg;
        arguments.push_back(std::move(arg));
        return ref;
    }

    inline void command_line::parse(std::span<const std::string> args)
    {
        const auto end = args.end();
        auto argv = args.begin();

        const auto parse_all = [](auto& args, auto& argv, const auto& end)
        {
            std::for_each(args.begin(), args.end(), 
                [&argv, &end](auto& arg) { argv = arg->parse(argv, end); });
            return argv;
        };
    
        if (std::distance(argv, end) > 0)
        {
            auto separator_found = false;
            ++argv;
            for (; argv != end && !separator_found; ++argv)
            {
                separator_found = detail::is_separator_tag(*argv);
                if (!separator_found)
                {
                    argv = parse_all(arguments, argv, end);
                }
            }

            if (!positional_arguments.empty())
            {
                for (argv = (!separator_found) ? std::next(args.begin()) : argv;
                    argv != end; ++argv)
                {
                    argv = parse_all(positional_arguments, argv, end);
                }
            }
        }

        detail::throw_on_invalid(arguments.cbegin(), arguments.cend());
        detail::throw_on_invalid(positional_arguments.cbegin(), positional_arguments.cend());
    }

    inline void command_line::parse(int argc, char** argv)
    {
        parse(std::vector<std::string>(argv, argv + argc));
    }

    inline void command_line::print_help(std::ostream& o)
    {
        o << name
            << ((!description.empty()) ? " - " + description : "")
            << "\n";
        std::for_each(std::begin(arguments), std::end(arguments),
              [&o](const auto& arg) { arg->print_help(o); });
        o << "\n";
    }

    inline void command_line::prevent_tag_args_after_positional_args()
    {
        if (!positional_arguments.empty())
        {
            throw std::logic_error("tag arguments cannot be given after positional arguments");
        }
    }
}
