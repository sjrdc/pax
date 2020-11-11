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

#include <algorithm>
#include <cctype>
#if __has_include(<format>)
#include <format>
#define PX_HAS_FORMAT
#endif
#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#if __has_include(<span>)
#include <span>
#define PX_HAS_SPAN
#endif
#include <string_view>
#include <vector>
#include <iterator>

namespace detail
{
    std::string pad_right(std::string_view s, size_t n)
    {
        return std::string(s).append(std::max(0u, n - s.size()), ' ');
    }

    template <typename T>
    T parse_scalar(const std::string& s)
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

    bool is_separator_tag(const std::string& s)
    {
        return s.size() == 2 && s[0] == '-' && s[1] == '-';
    }

    bool is_tag(const std::string& s)
    {
        return !s.empty() &&
            ((s[0] == '-' && s.size() > 1 && !std::isdigit(s[1])) ||
            (!is_separator_tag(s) && s[0] == '-' && s.size() > 2 && s[1] == '-'));
    }

    template <typename Iterator>
    Iterator find_invalid(const Iterator& begin, const Iterator& end)
    {
        return std::find_if_not(begin, end, [](auto& arg) { return arg->is_valid(); });
    }

    template <typename Iterator>
    void throw_on_invalid(const Iterator& begin, const Iterator& end)
    {
        if (Iterator invalid_arg = find_invalid(begin, end); invalid_arg != end)
        {
            throw std::runtime_error("argument '" + (*invalid_arg)->get_name() + "' invalid after parsing");
        }
    }
}

namespace px
{
    template <typename T>
    class scalar_storage
    {
    public:
        using value_type = T;
        bool has_value() const;
        const value_type& get_value() const;
        template <typename Iterator>
        Iterator parse(const Iterator& begin, const Iterator& end);

    private:
        std::optional<value_type> value = std::nullopt;
    };

    template <>
    class scalar_storage<bool>
    {
    public:
        using value_type = bool;
        bool has_value() const;
        const value_type& get_value() const;
        template <typename Iterator>
        Iterator parse(const Iterator& begin, const Iterator& end);
    private:
        bool value = false;
    };

    template <typename T>
    class multi_scalar_storage
    {
    public:
        using value_type = std::vector<T>;
        bool has_value() const;
        const value_type& get_value() const;

        template <typename Iterator>
        Iterator parse(const Iterator& begin, const Iterator& end);

    private:
       value_type value;
    };

#ifdef PX_HAS_SPAN
    using argv_iterator = std::span<const std::string>::iterator;
#else
    using argv_iterator = std::vector<std::string>::const_iterator;
#endif
    class argument
    {
    public:
        argument(std::string_view n);

        virtual ~argument() = default;
        virtual void print_help(std::ostream&) const = 0;
        virtual argv_iterator parse(const argv_iterator&, const argv_iterator&) = 0;
        virtual bool is_valid() const = 0;
        
        const std::string& get_name() const;
        const std::string& get_description() const;
        void set_description(std::string_view d);

    private:
        std::string name;
        std::string description;
    };
    
    template <typename T>
    class positional_argument : public argument
    {
    public:
        using value_type = T;
        using validation_function = std::function<bool(const value_type&)>;

        positional_argument(std::string_view n) :
            argument(n)
        {
        }

        virtual ~positional_argument<T>() = default;

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

    template <typename derived>
    class tag_argument : public argument
    {
    public:
        tag_argument(std::string_view n, std::string_view t) :
            argument(n),
            tag(t)
        {
        }

        virtual ~tag_argument() = default;

        const std::string& get_tag() const;

        const std::string& get_alternate_tag() const;
        derived& set_alternate_tag(std::string_view);

        const std::string& get_description() const;
        derived& set_description(std::string_view);

        void print_help(std::ostream& o) const override;

    protected:
        bool matches(std::string_view) const;

    private:
        derived& this_as_derived();

        std::string tag;
        std::string alternate_tag;
    };

    template <typename T, typename storage = scalar_storage<T>>
    class value_argument : public tag_argument<value_argument<T>>
    {
    public:
        using base = tag_argument<value_argument<T>>;
        using value_type = typename storage::value_type;
        using validation_function = std::function<bool(const value_type&)>;

        value_argument(std::string_view n, std::string_view t);

        virtual ~value_argument() = default;
        const value_type& get_value() const;
        value_argument<T, storage>& bind(value_type*);

        bool is_required() const;

        value_argument<T, storage>& set_required(bool);
        value_argument<T, storage>& set_validator(validation_function);

        bool is_valid() const override;
        void print_help(std::ostream&) const override;
        argv_iterator parse(const argv_iterator&, const argv_iterator&) override;

    private:
        storage value;
        value_type* bound_variable = nullptr;
        bool required = false;
        validation_function validator = [](const auto&) { return true; };
    };

    class flag_argument : public tag_argument<flag_argument>
    {
    public:
        using base = tag_argument<flag_argument>;

        flag_argument(std::string_view, std::string_view);
        virtual ~flag_argument() = default;

        flag_argument& bind(bool*);
        bool get_value() const;

        bool is_valid() const override;
        void print_help(std::ostream&) const override;
        argv_iterator parse(const argv_iterator&, const argv_iterator&) override;
    private:
        bool value = false;
        bool* bound_variable = nullptr;
    };

    class command_line
    {
    public:
        command_line(std::string_view program_name);

        flag_argument& add_flag_argument(std::string_view, std::string_view);
        template <typename T>
        value_argument<T>& add_value_argument(std::string_view, std::string_view);
        template <typename T>
        value_argument<T, multi_scalar_storage<T>>& add_multi_value_argument(std::string_view name, std::string_view);
        template <typename T>
        positional_argument<T>& add_positional_argument(std::string_view);

        void print_help(std::ostream&);
#ifdef PX_HAS_SPAN
        void parse(std::span<const std::string>);
#else
        void parse(const std::vector<std::string>&);
#endif
        void parse(int argc, char** argv);

    private:
        bool has_positional_arguments() const;
        void prevent_tag_args_after_positional_args();

        std::string name;
        std::string description;
        std::vector<std::shared_ptr<argument>> arguments;
        std::vector<std::shared_ptr<argument>> positional_arguments;
    };

    template <typename T>
    const typename scalar_storage<T>::value_type& scalar_storage<T>::get_value() const
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
    bool scalar_storage<T>::has_value() const
    {
        return value.has_value();
    }

    template <typename T>
    template <typename Iterator>
    Iterator scalar_storage<T>::parse(const Iterator& begin, const Iterator& end)
    {
        value = detail::parse_scalar<T>(*begin);
        return begin;
    }

    template <typename T>
    bool multi_scalar_storage<T>::has_value() const
    {
        return !std::empty(value);
    }

    template <typename T>
    const typename multi_scalar_storage<T>::value_type& multi_scalar_storage<T>::get_value() const
    {
        return value;
    }

    template <typename T>
    template <typename Iterator>
    Iterator multi_scalar_storage<T>::parse(const Iterator& begin, const Iterator& end)
    {
        auto i = begin;
        if (std::distance(begin, end))
        {
            for (; i != end && !detail::is_tag(*i); ++i)
            {
                value.push_back(detail::parse_scalar<T>(*i));
            }

            --i;
        }

        return i;
    }

    inline argument::argument(std::string_view n) :
        name(n)
    {
    }

    inline const std::string& argument::get_name() const
    {
        return name;
    }

    inline void argument::set_description(std::string_view d)
    {
        description = d;
    }

    inline const std::string& argument::get_description() const
    {
        return description;
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
            << get_name() << " "
            << get_description()
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
            throw std::runtime_error("getting value from invalid argument '" + get_name() + "'");
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

    template <typename T>
    bool tag_argument<T>::matches(std::string_view s) const
    {
        return (!tag.empty() && tag == s) || (!alternate_tag.empty() && alternate_tag == s);
    }

    template <typename T>
    void tag_argument<T>::print_help(std::ostream& o) const
    {
        constexpr auto alternate_tag_size = 15;
        o << "   "
            << tag
            << ((!alternate_tag.empty()) ?
                ", " + detail::pad_right(alternate_tag, alternate_tag_size - 2) :
                detail::pad_right("", alternate_tag_size))
            << get_description()
            << "\n";
    }

    template <typename T>
    const std::string& tag_argument<T>::get_description() const
    {
        return argument::get_description();
    }

    template <typename T>
    T& tag_argument<T>::set_description(std::string_view d)
    {
        argument::set_description(d);
        return this_as_derived();
    }

    template <typename T>
    const std::string& tag_argument<T>::get_tag() const
    {
        return tag;
    }

    template <typename T>
    const std::string& tag_argument<T>::get_alternate_tag() const
    {
        return alternate_tag;
    }

    template <typename T>
    T& tag_argument<T>::set_alternate_tag(std::string_view t)
    {
        alternate_tag = t;
        return this_as_derived();
    }

    template <typename T>
    T& tag_argument<T>::this_as_derived()
    {
        return *reinterpret_cast<T*>(this);
    }

    template <typename T, typename storage>
    value_argument<T, storage>::value_argument(std::string_view n, std::string_view t) :
        base(n, t)
    {
    }

    template <typename T, typename storage>
    value_argument<T, storage>& value_argument<T, storage>::bind(value_type* t)
    {
        bound_variable = t;
        return *this;
    }

    template <typename T, typename storage>
    bool value_argument<T, storage>::is_required() const
    {
        return required;
    }

    template <typename T, typename storage>
    value_argument<T, storage>& value_argument<T, storage>::set_required(bool b)
    {
        required = b;
        return *this;
    }

    template <typename T, typename storage>
    value_argument<T, storage>& value_argument<T, storage>::set_validator(typename value_argument<T, storage>::validation_function f)
    {
        validator = std::move(f);
        return *this;
    }

    template <typename T, typename storage>
    const typename value_argument<T, storage>::value_type& value_argument<T, storage>::get_value() const
    {
        if (!is_valid())
        {
            throw std::runtime_error("getting value from invalid argument '" + base::get_name() + "'");
        }
        return value.get_value();
    }

    template <typename T, typename storage>
    void value_argument<T, storage>::print_help(std::ostream& o) const
    {
        base::print_help(o);
    }

    template <typename T, typename storage>
    bool value_argument<T, storage>::is_valid() const
    {
        if (value.has_value())
        {
            return validator(value.get_value());
        }
        else return !required;
    }

    template <typename T, typename storage>
    argv_iterator
        value_argument<T, storage>::parse(const argv_iterator& begin,
            const argv_iterator& end)
    {
        if (std::distance(begin, end) > 1 && base::matches(*begin))
        {
            auto i = std::next(begin);
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

    flag_argument::flag_argument(std::string_view n, std::string_view t) :
        tag_argument<flag_argument>(n, t)
    {
    }

    void flag_argument::print_help(std::ostream& o) const
    {
        base::print_help(o);
    }

    flag_argument& flag_argument::bind(bool* b)
    {
        bound_variable = b;
        *bound_variable = value;
        return *this;
    }

    bool flag_argument::is_valid() const
    {
        return true;
    }

    argv_iterator
        flag_argument::parse(const argv_iterator& begin,
            const argv_iterator& end)
    {
        if (std::distance(begin, end) >= 1 && base::matches(*begin))
        {
            value = true;
            if (bound_variable != nullptr)
            {
                *bound_variable = value;
            }
        }

        return begin;
    }

    bool flag_argument::get_value() const
    {
        return value;
    }
    
    command_line::command_line(std::string_view program_name) :
        name(program_name)
    {
    }

    template <typename T>
    inline positional_argument<T>& command_line::add_positional_argument(std::string_view name)
    {
        auto arg = std::make_shared<positional_argument<T>>(name);
        positional_arguments.push_back(arg);
        return *arg;
    }

    inline flag_argument& command_line::add_flag_argument(std::string_view name, std::string_view tag)
    {
        prevent_tag_args_after_positional_args();
        auto arg = std::make_shared<flag_argument>(name, tag);
        arguments.push_back(arg);
        return *arg;
    }

    template <typename T>
    value_argument<T>& command_line::add_value_argument(std::string_view name, std::string_view tag)
    {
        prevent_tag_args_after_positional_args();
        auto arg = std::make_shared<value_argument<T>>(name, tag);
        arguments.push_back(arg);
        return *arg;
    }

    template <typename T>
    value_argument<T, multi_scalar_storage<T>>& command_line::add_multi_value_argument(std::string_view name, std::string_view tag)
    {
        prevent_tag_args_after_positional_args();
        auto arg = std::make_shared<value_argument<T, multi_scalar_storage<T>>>(name, tag);
        arguments.push_back(arg);
        return *arg;
    }

    
#ifdef PX_HAS_SPAN
    inline void command_line::parse(std::span<const std::string> args)
#else
    inline void command_line::parse(const std::vector<std::string>& args)
#endif
    {
        auto end = args.cend();
        auto argv = args.cbegin();

        bool separator_found = false;
        if (std::distance(argv, end) >= 1)
        {
            ++argv;
            for (; argv != end && !separator_found; ++argv)
            {
                separator_found = detail::is_separator_tag(*argv);
                for (auto& argument : arguments)
                {
                    argv = argument->parse(argv, end);
                }
            }

            detail::throw_on_invalid(arguments.cbegin(), arguments.cend());

            if (!separator_found)
            {
                argv = std::next(args.cbegin());
            }
            else
            {
            }

            for (; argv != end; ++argv)
            {
                for (auto& argument : positional_arguments)
                {
                    argv = argument->parse(argv, end);
                }
            }

            if (auto invalid_arg = detail::find_invalid(positional_arguments.cbegin(), positional_arguments.cend());
                invalid_arg != positional_arguments.cend())
            {
                throw std::runtime_error("invalid argument'" + (*invalid_arg)->get_name() + "' after parsing");
            }            
        }
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
        for (const auto& arg : arguments)
        {
            arg->print_help(o);
        }
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
