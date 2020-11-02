#include <iostream>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>
#include <span>

namespace sjrdc
{
    class argument
    {
    public:
	virtual void print_help(std::ostream&) const = 0;
    };
    
    template <typename T>
    class argument_base : public argument
    {
    public:
	argument_base(const std::string_view& n) : 
	    name(n)
	{
	}

	const std::string& get_name() const
	{
	    return name;
	}
	
	T& set_name(const std::string_view& n)
	{
	    name = n;
	    return this_as_derived();
	}

	const std::string& get_tag() const
	{
	    return tag;
	}
	
	T& set_tag(const std::string_view& t)
	{
	    tag = t;
	    return this_as_derived();
	}

	const std::string& get_long_tag() const
	{
	    return long_tag;
	}

	T& set_long_tag(const std::string_view& t)
	{
	    long_tag = t;
	    return this_as_derived();
	}
	
    private:
	T& this_as_derived()
	{
	    return *reinterpret_cast<T*>(this);
	}
	
	std::string name;
	std::string tag;
	std::string long_tag;
    };

    template <typename T>
    class value_argument : public argument_base<value_argument<T>>
    {
    public:
	using base = argument_base<value_argument<T>>;
	using value_type = T;

	value_argument(const std::string_view& n);
	T& get_value() const;
	void bind(T*);
	void set_default_value(T d);
	void print_help(std::ostream&) const override;
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
    void value_argument<T>::bind(T* t)
    {
	bound_variable = t;
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
    class multi_value_argument : public argument_base<multi_value_argument<T>>
    {
    public:
	using base = argument_base<multi_value_argument<T>>;
	using value_type = std::vector<T>;
	
	multi_value_argument(const std::string_view&);
	void print_help(std::ostream&) const override;
	value_type& get_value() const;
	void set_default_value(std::span<T>);
	void bind(std::vector<T>*);
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
    void multi_value_argument<T>::bind(std::vector<T>* v)
    {
	bound_variable = v;
    }
	
    class flag_argument : public argument_base<flag_argument>
    {
    public:
	using base = argument_base<flag_argument>;
	
	flag_argument(const std::string_view&);
	void bind(bool*);
	bool get_value() const;
	void print_help(std::ostream&) const override;
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

    void flag_argument::bind(bool* b)
    {
	bound_flag = b;
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

	void parse(std::span<std::string>);
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

    flag_argument& command_line::add_flag_argument(const std::string_view& name)
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

    void command_line::parse(std::span<std::string>)
    {
	std::cout << __PRETTY_FUNCTION__ << "\n";
    }

    void command_line::parse(int argc, char** argv)
    {
	std::vector<std::string> v(argv, argv + argc);
	parse(v);
    }
    
    void command_line::print_help(std::ostream& o)
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

int main(int argc, char** argv)
{
    std::cout << "Hello, world!\n";

    sjrdc::command_line cmd(argv[0]);
    cmd.add_flag_argument("piet");
    cmd.add_value_argument<int>("some integer")
	.set_tag("-i")
	.set_long_tag("--integer");
    cmd.print_help(std::cout);
    cmd.parse(argc, argv);
    return 0;
}
