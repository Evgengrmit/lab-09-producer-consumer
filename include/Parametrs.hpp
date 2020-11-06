// Copyright 2020 Your Name <your_email>
#ifndef INCLUDE_PARAMETRS_HPP_
#define INCLUDE_PARAMETRS_HPP_

#include <atomic>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>

struct Parametrs {
  std::string url;
  std::string output;
  unsigned depth;
  unsigned downloader_threads;
  unsigned parser_threads;
};
Parametrs parametrs;
namespace po = boost::program_options;
// Обработка аргументов командной строки
void prepare_command_line(int argc, char* argv[]) {
  po::options_description desc{"Options"};
  desc.add_options()("url", po::value<std::string>())(
      "depth", po::value<unsigned>())("network_threads", po::value<unsigned>())(
      "parser_threads", po::value<unsigned>())("output",
                                               po::value<std::string>());
  po::variables_map vm;
  store(parse_command_line(argc, argv, desc), vm);
  notify(vm);
  if (vm.count("url")) parametrs.url = vm["url"].as<std::string>();
  if (vm.count("output")) parametrs.output = vm["output"].as<std::string>();
  if (vm.count("depth")) parametrs.depth = vm["depth"].as<unsigned>();
  if (vm.count("network_threads"))
    parametrs.downloader_threads = vm["network_threads"].as<unsigned>();
  if (vm.count("parser_threads"))
    parametrs.parser_threads = vm["parser_threads"].as<unsigned>();
}
void printParametrs() {
  std::cout << "URL: " << parametrs.url << std::endl;
  std::cout << "Depth: " << parametrs.depth << std::endl;
  std::cout << "Network threads: " << parametrs.downloader_threads << std::endl;
  std::cout << "Parser threads: " << parametrs.parser_threads << std::endl;
  std::cout << "Output file: " << parametrs.output << std::endl;
}
#endif  // INCLUDE_PARAMETRS_HPP_
