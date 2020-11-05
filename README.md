# px
px is a command line argument parser written in modern C++.
## why?
Don't we have enough command line argument parsers already? Probably, yes...
This is meant to be an excercise in the use of some C++17 and C++20 features, as well as a way to provide a parser that
- adheres to the DRY principle
- facilitates the use of the filesystem library
- by no means aims to provide many different ways to specify arguments; one simple syntax is sufficient

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
  cli.add_value_argument<int>("integer", "-i")
    .set_description("the number of kittens to show")
    .set_validator([](auto i) { return i > 0 && i <= 5; });
    .bind(&i);
  cli.add_value_argument<fs::path>("path", "-p")
    .set_required(true)
    .set_description("the path to use for storage of the shown kittens")
    .bind(&pth)
    .set_validator([](auto p) { return fs::exists(p) && fs::is_regular_file(p); });
  auto& help_arg = cli.add_flag_argument("help", "-h")
    .set_alternate_tag("--help");
  
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
