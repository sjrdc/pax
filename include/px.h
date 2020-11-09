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

    bool is_tag(const std::string& s)
    {
        return !s.empty() &&
            ((s[0] == '-' && s.size() > 1 && !std::isdigit(s[1])) ||
            (s[0] == '-' && s.size() > 2 && s[1] == '-'));
    }
}

namespace px
{
#ifdef PX_HAS_SPAN
    using argv_iterator = std::span<const std::string>::iterator;
#else
    using argv_iterator = std::vector<std::string>::const_iterator;
#endif
    class argument
    {
    public:
        virtual void print_help(std::ostream&) const = 0;
        virtual argv_iterator parse(const argv_iterator&, const argv_iterator&) = 0;
        virtual bool is_valid() const = 0;
    };

    template <typename derived>
    class tag_argument : public argument
    {
    public:
        tag_argument(std::string_view n, std::string_view t) :
            name(n),
            tag(t)
        {
        }

        const std::string& get_name() const;
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

        std::string name;
        std::string description;
        std::string tag;
        std::string alternate_tag;
    };

    template <typename T>
    bool tag_argument<T>::matches(std::string_view s) const
    {
        return (!tag.empty() && tag == s) || (!alternate_tag.empty() && alternate_tag == s);
    }

    template <typename T>
    const std::string& tag_argument<T>::get_name() const
    {
        return name;
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
            << description
            << "\n";
    }

    template <typename T>
    const std::string& tag_argument<T>::get_description() const
    {
        return description;
    }

    template <typename T>
    T& tag_argument<T>::set_description(std::string_view d)
    {
        description = d;
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

    template <typename T>
    class value_argument : public tag_argument<value_argument<T>>
    {
    public:
        using base = tag_argument<value_argument<T>>;
        using value_type = T;

        value_argument(std::string_view n, std::string_view t);

        const value_type& get_value() const;
        value_argument<T>& bind(T*);

        bool is_required() const;
        value_argument<T>& set_required(bool);

        value_argument<T>& set_validator(std::function<bool(const T&)>);

        bool is_valid() const override;
        void print_help(std::ostream&) const override;
        argv_iterator parse(const argv_iterator&, const argv_iterator&) override;

    private:
        std::optional<value_type> value = std::nullopt;
        value_type* bound_variable = nullptr;
        bool required = false;
        std::function<bool(const value_type&)> validator = [](const auto&) { return true; };
    };

    template <typename T>
    value_argument<T>::value_argument(std::string_view n, std::string_view t) :
        base(n, t)
    {
    }

    template <typename T>
    value_argument<T>& value_argument<T>::bind(T* t)
    {
        bound_variable = t;
        return *this;
    }

    template <typename T>
    bool value_argument<T>::is_required() const
    {
        return required;
    }

    template <typename T>
    value_argument<T>& value_argument<T>::set_required(bool b)
    {
        required = b;
        return *this;
    }

    template <typename T>
    value_argument<T>& value_argument<T>::set_validator(std::function<bool(const T&)> f)
    {
        validator = std::move(f);
        return *this;
    }

    template <typename T>
    const typename value_argument<T>::value_type& value_argument<T>::get_value() const
    {
        if (value)
        {
            return *value;
        }
        else
        {
            throw std::runtime_error("argument '" + base::get_name() + "' does not have a value");
        }
    }

    template <typename T>
    void value_argument<T>::print_help(std::ostream& o) const
    {
        base::print_help(o);
    }

    template <typename T>
    bool value_argument<T>::is_valid() const
    {
        if (value.has_value())
        {
            return validator(*value);
        }
        else return !required;
    }

    template <typename T>
    argv_iterator
        value_argument<T>::parse(const argv_iterator& begin,
            const argv_iterator& end)
    {
        if (std::distance(begin, end) > 1 && base::matches(*begin))
        {
            auto i = std::next(begin);
            value = detail::parse_scalar<value_type>(*i);
            if (bound_variable != nullptr)
            {
                *bound_variable = *value;
            }
            return i;
        }
        else
        {
            return begin;
        }
    }

    template <typename T>
    class multi_value_argument : public tag_argument<multi_value_argument<T>>
    {
    public:
        using base = tag_argument<multi_value_argument<T>>;
        using value_type = std::vector<T>;

        multi_value_argument(std::string_view, std::string_view);
        void print_help(std::ostream&) const override;
        const value_type& get_value() const;
        multi_value_argument<T>& bind(std::vector<T>*);

        bool is_required() const;
        multi_value_argument<T>& set_required(bool);
        multi_value_argument<T>& set_validator(std::function<bool(const std::vector<T>&)>);
        bool is_valid() const override;
        argv_iterator parse(const argv_iterator&, const argv_iterator&) override;
    private:
        bool required = false;
        value_type value;
        value_type* bound_variable = nullptr;
        std::function<bool(const std::vector<T>&)> validator = [](const auto&) { return true; };
    };

    template <typename T>
    multi_value_argument<T>::multi_value_argument(std::string_view n, std::string_view t) :
        base(n, t)
    {
    }

    template <typename T>
    multi_value_argument<T>& multi_value_argument<T>::bind(std::vector<T>* v)
    {
        bound_variable = v;
    }

    template <typename T>
    const std::vector<T>& multi_value_argument<T>::get_value() const
    {
        return value;
    }

    template <typename T>
    bool multi_value_argument<T>::is_required() const
    {
        return required;
    }

    template <typename T>
    multi_value_argument<T>& multi_value_argument<T>::set_required(bool b)
    {
        required = b;
        return *this;
    }

    template <typename T>
    multi_value_argument<T>& multi_value_argument<T>::set_validator(std::function<bool(const std::vector<T>&)> f)
    {
	validator = f;
	return *this;
    }

    template <typename T>
    bool multi_value_argument<T>::is_valid() const
    {
        if (required)
	{
	    return !value.empty() && validator(value);
	}
	else return true;
    }

    template <typename T>
    argv_iterator multi_value_argument<T>::parse(const argv_iterator& begin, const argv_iterator& end)
    {
        auto i = begin;
        if (std::distance(begin, end) > 1 && base::matches(*begin))
        {
            for (i = std::next(i); i != end && !detail::is_tag(*i); ++i)
            {
		value.push_back(detail::parse_scalar<T>(*i));
            }

            if (bound_variable != nullptr)
            {
                *bound_variable = value;
            }
            --i;
        }

        return i;
    }

    class flag_argument : public tag_argument<flag_argument>
    {
    public:
        using base = tag_argument<flag_argument>;

        flag_argument(std::string_view, std::string_view);
        flag_argument& bind(bool*);
        bool get_value() const;

        bool is_valid() const override;
        void print_help(std::ostream&) const override;
        argv_iterator parse(const argv_iterator&, const argv_iterator&) override;
    private:
        bool value = false;
        bool* bound_flag = nullptr;
    };

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
        bound_flag = b;
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
        }
        return begin;
    }

    bool flag_argument::get_value() const
    {
        return value;
    }

    class command_line
    {
    public:
        command_line(std::string_view program_name);

        flag_argument& add_flag_argument(std::string_view, std::string_view);
        template <typename T>
        value_argument<T>& add_value_argument(std::string_view, std::string_view);
        template <typename T>
        multi_value_argument<T>& add_multi_value_argument(std::string_view name, std::string_view);

        void print_help(std::ostream&);
#ifdef PX_HAS_SPAN
        void parse(std::span<const std::string>);
#else
        void parse(const std::vector<std::string>&);
#endif
        void parse(int argc, char** argv);

    private:
        std::string name;
        std::string description;
        std::vector<std::shared_ptr<argument>> arguments;
    };

    template <typename T>
    void multi_value_argument<T>::print_help(std::ostream& o) const
    {
        o << tag_argument<multi_value_argument<T>>::get_name() << "\n";
    }

    command_line::command_line(std::string_view program_name) :
        name(program_name)
    {
    }

    inline flag_argument& command_line::add_flag_argument(std::string_view name, std::string_view tag)
    {
        auto arg = std::make_shared<flag_argument>(name, tag);
        arguments.push_back(arg);
        return *arg;
    }

    template <typename T>
    value_argument<T>& command_line::add_value_argument(std::string_view name, std::string_view tag)
    {
        auto arg = std::make_shared<value_argument<T>>(name, tag);
        arguments.push_back(arg);
        return *arg;
    }

    template <typename T>
    multi_value_argument<T>& command_line::add_multi_value_argument(std::string_view name, std::string_view tag)
    {
        auto arg = std::make_shared<multi_value_argument<T>>(name, tag);
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
        for (auto argv = args.begin(); argv != end; ++argv)
        {
            for (auto& argument : arguments)
            {
                argv = argument->parse(argv, end);
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
}
