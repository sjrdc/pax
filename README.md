<!-- PROJECT SHIELDS -->
<!--
*** I'm using markdown "reference style" links for readability.
*** Reference links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom of this document for the declaration of the reference variables
*** for contributors-url, forks-url, etc. This is an optional, concise syntax you may use.
*** https://www.markdownguide.org/basic-syntax/#reference-style-links
-->
[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]
[![LinkedIn][linkedin-shield]][linkedin-url]


<!-- PROJECT LOGO -->
<br />
<p align="center">
  <a href="https://github.com/github_username/repo_name">
    <img src="images/logo.png" alt="Logo" width="80" height="80">
  </a>

  <h3 align="center">YOUR_TITLE</h3>

  <p align="center">
    YOUR_SHORT_DESCRIPTION
    <br />
    <a href="https://github.com/github_username/repo_name"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="https://github.com/github_username/repo_name">View Demo</a>
    ·
    <a href="https://github.com/github_username/repo_name/issues">Report Bug</a>
    ·
    <a href="https://github.com/github_username/repo_name/issues">Request Feature</a>
  </p>
</p>



<!-- TABLE OF CONTENTS -->
## Table of Contents

* [About the Project](#about-the-project)
  * [Built With](#built-with)
* [Getting Started](#getting-started)
  * [Prerequisites](#prerequisites)
  * [Installation](#installation)
* [Usage](#usage)
* [Roadmap](#roadmap)
* [Contributing](#contributing)
* [License](#license)
* [Contact](#contact)
* [Acknowledgements](#acknowledgements)



<!-- ABOUT THE PROJECT -->
## About The Project

# px
px is a command line argument parser written in modern C++.
## why?
Don't we have enough command line argument parsers already? Probably, yes...
This is meant to be an excercise in the use of some C++17 and C++20 features, as well as a way to provide a parser that
- adheres to the DRY principle; in many of these libraries, we need to first provide names to arguments, to later retrieve values with the same name, or assign some default, then validate the retrieved value. All of this could be a lot leaner.
- facilitates the use of the filesystem library
- by no means aims to provide many different ways to specify arguments; one simple syntax is sufficient

### Built With

* love
* standard C++20
* cmake

<!-- GETTING STARTED -->
## Getting Started

To get a local copy up and running follow these simple steps.

### Prerequisites

This is an example of how to list things you need to use the software and how to install them.
* npm
```sh
npm install npm@latest -g
```

### Installation

1. Clone the repo
```sh
git clone https://github.com/github_username/repo_name.git
```
2. Install NPM packages
```sh
npm install
```

<!-- USAGE EXAMPLES -->
## Usage
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
    .set_description("the number of kittens to show; must be large than 0 and 5 at most")
    .set_validator([](auto i) { return i > 0 && i <= 5; })
    .bind(&i);
  cli.add_value_argument<fs::path>("path", "-p")
    .set_required(true)
    .set_description("the path to use for storage of the shown kittens (must be an existing file)")
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
  store_kittens(pth);
  
  return 0;
}
```

<!-- ROADMAP -->
## Roadmap

See the [open issues](https://github.com/sjrdc/px/issues) for a list of proposed features (and known issues).

<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to be learn, inspire, and create. Any contributions you make are **greatly appreciated**.

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<!-- LICENSE -->
## License

Distributed under the GPLv3 License. See `LICENSE` for more information.

<!-- ACKNOWLEDGEMENTS -->
## Acknowledgements

* []()
* []()
* []()


<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/sjrdc/px.svg?style=flat-square
[contributors-url]: https://github.com/sjrdc/px/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/sjrdc/px.svg?style=flat-square
[forks-url]: https://github.com/sjrdc/px/network/members
[stars-shield]: https://img.shields.io/github/stars/sjrdc/px.svg?style=flat-square
[stars-url]: https://github.com/sjrdc/px/stargazers
[issues-shield]: https://img.shields.io/github/issues/sjrdc/px.svg?style=flat-square
[issues-url]: https://github.com/sjrdc/px/issues
[license-shield]: https://img.shields.io/github/license/sjrdc/px.svg?style=flat-square
[license-url]: https://github.com/sjrdc/px/blob/main/LICENSE
