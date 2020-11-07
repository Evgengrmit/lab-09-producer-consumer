// Copyright 2020 Your Name <your_email>

#include "Downloader.hpp"
#include "Parametrs.hpp"
#include "Parser.hpp"

void Run_crauler(int argc, char* argv[]){
  prepare_command_line(argc, argv);
  printParametrs();
  Downloader d(parametrs.downloader_threads);
  Parser p(parametrs.parser_threads);
  d.downloading(parametrs.url, parametrs.depth, p);
}
//int main(int argc, char* argv[]) {
//  Run_crauler(argc,argv);
//  return 0;
//}

/*
 ./cmake-build-debug/tests --url https://www.bbc.com/russian/news-54840425 --depth 3 --network_threads 4 --parser_threads 4 --output /Users/evgenii/CLionProjects/lab-09-producer-consumer/output.txt
 */
