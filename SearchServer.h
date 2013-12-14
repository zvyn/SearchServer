// Copyright 2012, Milan Oberkirch
// Author: Milan Oberkirch <oberkirm@informatik.uni-freiburg.de>

#ifndef SEARCHSERVER_H_
#define SEARCHSERVER_H_

#include <gtest/gtest.h>
#include <unistd.h>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <fstream>  // NOLINT
#include <stdexcept>
#include <string>
#include <vector>
#include "./InvertedIndex.h"
#include "./QueryProcessor.h"

using boost::asio::ip::tcp;
namespace po = boost::program_options;

class SearchServer {
  InvertedIndex _invertedIndex;
  QueryProcessor _queryProcessor;

  string _usage;
  int8_t _minArgs;
  int8_t _maxArgs;
  po::variables_map _optionVariables;

  string _file;
  unsigned int _port;
  string _webRoot;

  unsigned int _k;
  size_t _numberOfResults;
  float _bm25k;
  float _bm25b;

 public:
  void parse(int argc, char** argv);
  void run();

 private:
  class Error404 : public std::runtime_error  {
   public:
    explicit Error404(string path) : std::runtime_error(
        "404 - '" + path + "' not found.") {}
  };
  class Error501: public std::runtime_error  {
   public:
    explicit Error501(string message) : std::runtime_error(message) {}
  };

  // Add HTTP-headers to strings.
  string http200(string const& filePath);
  string http200(string const& content, string const& mimeType);
  string http418(string const& content);
  // Set the Options read by parse
  void setOptions();
  // Compute the default for maxEditDistance (ceil(|w|/5))
  size_t maxEditDistance(string const& query);
  // Server-loop
  void runServer();
  // Replace URL-Codes
  void replaceUrlCodes(string* request);
  // Extract value of an query
  string getValue(string const& request, string const& name) const;
  // Extract path of an query check if file exists.
  string getFilePath(string const& request) const;
};

#endif  // SEARCHSERVER_H_
