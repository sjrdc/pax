#include "pax.h"

int main(int argc, char** argv)
{
    std::cout << "Hello, world!\n";

    pax::command_line cmd(argv[0]);
    cmd.add_flag_argument("piet");
    cmd.add_value_argument<int>("some integer")
	.set_tag("-i")
	.set_long_tag("--integer");
    cmd.print_help(std::cout);
    cmd.parse(argc, argv);
    
    return 0;
}
