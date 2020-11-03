# px
px is a command line argument parser written in modern C++.

## example usage
```c++
#include "px.h"

int main(int argc, char** argv)
{
  int i = 1;
  
  px::command_line cli("the program name");
  cli.add_value_argument<int>("integer")
    .set_tag("-i")
    .set_description("the number of kittens to show")
    .set_validator([](auto i) { return i > 0 && i <= 5; });
    .bind(&i);
  auto& help_arg = cli.add_flag_argument("help")
    .set_tag("-h")
    .set_long_tag("--help");
  
  cli.parse(argc, argv);
  if (help_arg.get_value())
  {
    cli.print_help(std::cout);
    return 0;
  }
  
  show_kittens(i);
  
  return 0;
}
```
