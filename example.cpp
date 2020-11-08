#include "px.h"
#include <filesystem>
#include <iostream>

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
  auto& help_arg = cli.add_flag_argument("help", "-h")
      .set_alternate_tag("--help")
      .set_description("show this message");
  cli.add_value_argument<int>("integer", "-i")
    .set_description("the number of kittens to show; must be large than 0 and 5 at most")
    .set_validator([](auto i) { return i > 0 && i <= 5; })
    .bind(&i);
  cli.add_value_argument<fs::path>("path", "-p")
    .set_required(true)
    .set_description("the path to use for storage of the shown kittens (must be an existing file)")
    .bind(&pth)
    .set_validator([](auto p) { return fs::exists(p) && fs::is_regular_file(p); });

  
  cli.parse(argc, argv);
  if (help_arg.get_value())
  {
    cli.print_help(std::cout);
    return 0;
  }
  
  show_kittens(i);
  store_kittens(pth);
  
  return 0;
}
