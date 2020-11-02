#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string_view>
#include <vector>
#include <span>

namespace pax
{
    class argument
    {
    public:
	virtual void print_help(std::ostream&) const = 0;
	virtual void parse(std::span<const std::string>::iterator&, 
			   const std::span<const std::string>::iterator&) = 0;
    };
    
    template <typename T>
    class argument_base : public argument
    {
    public:
	argument_base(const std::string_view& n) : 
	    name(n)
	{
	}
	const std::string& get_name() const;
	T& set_name(const std::string_view&);

	const std::string& get_tag() const;
	T& set_tag(const std::string_view&);

	const std::string& get_long_tag() const;
	T& set_long_tag(const std::string_view&);

    private:
	T& this_as_derived();
	
	std::string name;
	std::string tag;
	std::string long_tag;
    };

    template <typename T>
    const std::string& argument_base<T>::get_name() const
    {
	return name;
    }
    
    template <typename T>
    T& argument_base<T>::set_name(const std::string_view& n)
    {
	name = n;
	return this_as_derived();
    }
    
    template <typename T>
    const std::string& argument_base<T>::get_tag() const
    {
	return tag;
    }
    
    template <typename T>
    T& argument_base<T>::set_tag(const std::string_view& t)
    {
	tag = t;
	return this_as_derived();
    }
    
    template <typename T>
    const std::string& argument_base<T>::get_long_tag() const
    {
	return long_tag;
    }
    
    template <typename T>
    T& argument_base<T>::set_long_tag(const std::string_view& t)
    {
	long_tag = t;
	return this_as_derived();
    }

    template <typename T>
    T& argument_base<T>::this_as_derived()
    {
	return *reinterpret_cast<T*>(this);
    }

    template <typename T>
    class value_argument : public argument_base<value_argument<T>>
    {
    public:
	using base = argument_base<value_argument<T>>;
	using value_type = T;

	value_argument(const std::string_view& n);
	const T& get_value() const;
	value_argument<T>& bind(T*);
	void set_default_value(T d);
	void print_help(std::ostream&) const override;
	void parse(std::span<const std::string>::iterator&, 
		   const std::span<const std::string>::iterator&) override;
    private:
	value_type value{};
	std::optional<value_type> default_value;
	value_type* bound_variable = nullptr;
    };

    template <typename T>
    value_argument<T>::value_argument(const std::string_view& n) :
	base(n)
    {
    }

    template <typename T>
    value_argument<T>& value_argument<T>::bind(T* t)
    {
	bound_variable = t;
	return *this;
    }

    template <typename T>
    const typename value_argument<T>::value_type& value_argument<T>::get_value() const
    {
	return value;
    }

    template <typename T>
    void value_argument<T>::set_default_value(T d)
    {
	default_value = d;
    }

    template <typename T>
    void value_argument<T>::print_help(std::ostream& o) const
    {
	o << base::get_name() << "\n";
    }

    template <typename T>
    void value_argument<T>::parse(std::span<const std::string>::iterator& begin, 
		   const std::span<const std::string>::iterator& end)
    {
	if (std::distance(begin, end) > 1 && (*begin == base::get_tag() || *begin == base::get_long_tag()))
	{
	    ++begin;
	    if constexpr (std::is_same_v<std::string, T>)
	    {
		value = *begin;
	    }
	    else
	    {
		std::istringstream stream(*begin);
		T t;
		stream >> t;
		if (!stream.eof() || stream.fail())
		{
		    throw std::runtime_error("could not parse from '" + *begin + "'");
		}
		value = t;
	    }
	}
    }
    
    template <typename T>
    class multi_value_argument : public argument_base<multi_value_argument<T>>
    {
    public:
	using base = argument_base<multi_value_argument<T>>;
	using value_type = std::vector<T>;
	
	multi_value_argument(const std::string_view&);
	void print_help(std::ostream&) const override;
	value_type& get_value() const;
	void set_default_value(std::span<T>);
	multi_value_argument<T>& bind(std::vector<T>*);
	void parse(std::span<const std::string>::iterator&, 
		   const std::span<const std::string>::iterator&) override;
    private:
	value_type& value;
	std::optional<value_type> default_value;
	value_type* bound_variable = nullptr;
    };

    template <typename T>
    multi_value_argument<T>::multi_value_argument(const std::string_view& n) :
	base(n)
    {
    }

    template <typename T>
    void multi_value_argument<T>::set_default_value(std::span<T> v)
    {
	default_value->assign(std::begin(v), std::end(v));
    }

    template <typename T>
    multi_value_argument<T>& multi_value_argument<T>::bind(std::vector<T>* v)
    {
	bound_variable = v;
    }

    template <typename T>
    void multi_value_argument<T>::parse(std::span<const std::string>::iterator&, 
					const std::span<const std::string>::iterator&)
    {
    }

    class flag_argument : public argument_base<flag_argument>
    {
    public:
	using base = argument_base<flag_argument>;
	
	flag_argument(const std::string_view&);
	flag_argument& bind(bool*);
	bool get_value() const;
	void print_help(std::ostream&) const override;
	void parse(std::span<const std::string>::iterator&, 
		   const std::span<const std::string>::iterator&) override;
    private:
	bool value = false;
	bool* bound_flag = nullptr;
    };

    flag_argument::flag_argument(const std::string_view& n) :
	argument_base<flag_argument>(n)
    {
    }
    
    void flag_argument::print_help(std::ostream& o) const
    {
	o << get_name() << "\n";
    }

    flag_argument& flag_argument::bind(bool* b)
    {
	bound_flag = b;
	return *this;
    }

    void flag_argument::parse(std::span<const std::string>::iterator& begin, 
			      const std::span<const std::string>::iterator& end)
    {
	if (std::distance(begin, end) >= 1 && 
	    (*begin == base::get_tag() || *begin == base::get_long_tag()))
	{
	    value = true;
	}
    }

    bool flag_argument::get_value() const
    {
	return value;
    }

    class command_line
    {
    public:
	command_line(const std::string_view& program_name);
	
	flag_argument& add_flag_argument(const std::string_view& name);
	template <typename T>
	value_argument<T>& add_value_argument(const std::string_view& name);
	template <typename T>
	multi_value_argument<T>& add_multi_value_argument(const std::string_view& name);

	void print_help(std::ostream&);

	void parse(std::span<const std::string>);
	void parse(int argc, char** argv);

    private:
	std::string name;
	std::string description;
	std::vector<std::shared_ptr<argument>> arguments;
    };
    
    template <typename T>
    void multi_value_argument<T>::print_help(std::ostream& o) const
    {
	o << argument_base<multi_value_argument<T>>::get_name() <<"\n";
    }

    command_line::command_line(const std::string_view& program_name) :
	name(program_name)
    {
    }

    inline flag_argument& command_line::add_flag_argument(const std::string_view& name)
    {
	auto arg = std::make_shared<flag_argument>(name);
	arguments.push_back(arg);
	return *arg;
    }

    template <typename T>
    value_argument<T>& command_line::add_value_argument(const std::string_view& name) 
    {
	auto arg = std::make_shared<value_argument<T>>(name);
	arguments.push_back(arg);
	return *arg;
    }

    template <typename T>
    multi_value_argument<T>& command_line::add_multi_value_argument(const std::string_view& name) 
    {
	auto arg = std::make_shared<multi_value_argument<T>>(name);
	arguments.push_back(arg);
	return *arg;
    }

    inline void command_line::parse(std::span<const std::string> args)
    {
	auto end = args.cend();
	for (auto argv = args.cbegin(); argv != end; ++argv)
	{
	    for (auto& argument : arguments)
	    {
		argument->parse(argv, end);
	    }
	}
    }

    inline void command_line::parse(int argc, char** argv)
    {
	std::vector<std::string> v(argv, argv + argc);
	parse(v);
    }
    
    inline void command_line::print_help(std::ostream& o)
    {
	o << name 
	  << ((!description.empty()) ?  " - " + description : "")
	  << "\n";
	for (const auto& arg : arguments)
	{
	    arg->print_help(o);
	}
    }
}
