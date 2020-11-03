# px
px is a command line argument parser written in modern C++.

## example usage
```c++
#include "px.h"
#include <filesystem>

namespace fs = std::filesystem;

void show_kittens(int i)
{
}

void store_kittens(const fs::path& p)
{
}

int main(int argc, char** argv)
{
  int i = 1;
  fs::path pth;
  
  px::command_line cli("the program name");
  cli.add_value_argument<int>("integer")
    .set_tag("-i")
    .set_description("the number of kittens to show")
    .set_validator([](auto i) { return i > 0 && i <= 5; });
    .bind(&i);
  cli.add_value_argument<fs::path>("path")
    .set_tag("-p")
    .set_required(true)
    .set_description("the path to use for storage of the shown kittens")
    .bind(&pth)
    .set_validator([](auto p) { return fs::exists(p) && fs::is_regular_file(p); });
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
  store_kittens(p);
  
  return 0;
}
```
