// Copyright 2012, Milan Oberkirch
// Author: Milan Oberkirch <oberkirm@informatik.uni-freiburg.de>

#include "./SearchServer.h"
#include <unistd.h>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <numeric>
#include <algorithm>
#include <ctime>
#include <map>
#include <string>
#include <vector>
#include <stdexcept>
#include "./InvertedIndex.h"
#include "./QueryProcessor.h"

using std::cout;
using std::endl;
using std::flush;
using std::cerr;
using std::map;
using boost::asio::ip::tcp;
using std::string;
using std::ifstream;
using std::cout;
namespace po = boost::program_options;

// ___________________________________________________________________________
string SearchServer::http200(
    string const& content, string const& mimeType) {
  std::stringstream answer;
  answer << "HTTP/1.0 200 OK\r\n"
    << "Server: None\r\n"
    << "Content-Length: " << content.size() << "\r\n"
    << "Content-Language: en\r\n"
    << "Content-Type: " << mimeType  << "; charset=utf-8\r\n"
    << "Connection: close\r\n"
    << "\r\n"
    << content;
  return answer.str();
}

// ___________________________________________________________________________
string SearchServer::http200(
    string const& filePath) {
  string suffix = filePath.substr(filePath.rfind('.') + 1);
  std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);

  if (suffix == "py" || suffix == "php") {
    string cmd;
    if (suffix == "php")
      string cmd = "php " + filePath;
    else
      cmd = "python " + filePath;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while (!feof(pipe)) {
      if (fgets(buffer, 128, pipe) != NULL)
        result += buffer;
    }
    pclose(pipe);
    return http200(result, "text/html");
  }

  map<string, string> mimeTypes = {
    {"jpg", "image/jpeg"},
    {"png", "image/png"},
    {"ico", "image/image/vnd.microsoft.icon"},
    {"html", "text/html"},
    {"css", "text/css"},
    {"less", "text/x-less"},
    {"js", "application/javascript"}
  };

  string mimeType = mimeTypes[suffix];
  if (mimeType.empty())
    throw Error501("Unknown Filetype \"" + suffix + "\"");

  std::ifstream file(filePath.c_str());
  string content;

  file.seekg(0, std::ios::end);
  content.reserve(file.tellg());
  file.seekg(0, std::ios::beg);

  content.assign((std::istreambuf_iterator<char>(file)),
      std::istreambuf_iterator<char>());

  return http200(content, mimeType);
}

// ___________________________________________________________________________
string SearchServer::http418(
    string const& content = "") {
  std::stringstream contentStream;
  contentStream << "<!DOCTYPE html>" << endl
    << "<html>"
    << "<head><title>418 - I'm a teapot</title></head>"
    << "<body><h1>418 - I'm a teapot</h1>"
    << "<p>Someone filled coffe into a"
    << " teapot, sorry.</br>"
    << content
    << "</p></body></html>";
  std::stringstream answer;
  answer << "HTTP/1.0 418 I'm a teapot\r\n"
    << "Content-Length: " << contentStream.str().size() << "\r\n"
    << "Content-Type: text/html" << "\r\n"
    << "Connection: close\r\n"
    << "\r\n"
    << contentStream.str();
  return answer.str();
}

// ___________________________________________________________________________
void SearchServer::parse(int argc, char** argv) {
  // Number of Arguments expected:
  int8_t _minArgs = 2;
  int8_t _maxArgs = 6;
  std::stringstream defaults;
  defaults << "Defaults:"
    << "\n\tweb-root = " << (_webRoot = "www")
    << "\n\tk-gram-length = " << (_k = 3)
    << "\n\tnumber-of-results = " << (_numberOfResults = 10)
    << "\n\tbm25k = " << (_bm25k = 1.75)
    << "\n\tbm25b = " << (_bm25b = 0.75)
    << endl;

  string optionsPrefix =
    "Usage: ./ApproximateMatchingMain <input-file> <port> [Options]\n" +
    string("Options are");

  po::options_description allOptions("");
  po::options_description hiddenOptions("");
  po::options_description visibleOptions(optionsPrefix);
  po::options_description generalOptions("General Options");
  po::options_description searchOptions("Search");
  po::options_description editDistanceOptions("Edit-Distance");

  generalOptions.add_options()
    ("help,h", "Show this message and exit")
    ("web-root,w", po::value<string>(), "Path to folder with files to serve.");
  editDistanceOptions.add_options()
    ("k-gram-length,k", po::value<unsigned int>(),
     "The k from k-gram. See http://en.wikipedia.org/wiki/N-gram.");
  searchOptions.add_options()
    ("results,r", po::value<size_t>(),
     "Set number of results to send to client.")
    ("bm25b,b", po::value<float>(), "Set the b-value of the BM25-Algorithm.")
    ("bm25k,n", po::value<float>(), "Set the k-value of the BM25-Algorithm.");
  hiddenOptions.add_options()
    ("input-file", po::value<string>(), "(CSV-)File with data to search in.")
    ("port,p", po::value<unsigned int>(), "Port to listen on");

  visibleOptions
    .add(generalOptions)
    .add(editDistanceOptions)
    .add(searchOptions);
  allOptions.add(visibleOptions).add(hiddenOptions);

  po::positional_options_description positionalOptions;
  positionalOptions.add("input-file", 1);
  positionalOptions.add("port", 1);

  std::stringstream usage;
  usage << "HTTP-Server to serve files in web-root and answer queries for"
    << "words or records from an CSV-file with lines of the kind"
    << "\"url\\ttext\" (Queries have a 'v=arg' and/or 'q=arg' in the"
    << "GET-field)." << std::endl
    << visibleOptions << std::endl << defaults.str() << std::endl;
  _usage = usage.str();

  try {
    po::store(po::command_line_parser(argc, argv).
        options(allOptions).positional(positionalOptions).run(),
        _optionVariables);
    po::notify(_optionVariables);
    if (_optionVariables.count("help") || argc == 1) {
      std::cout << _usage;
      exit(0);
    }
    if (argc < _minArgs || argc > _maxArgs )
      throw po::error("Wrong number of arguments.");
    setOptions();
  } catch(std::exception const &e) {
    std::cerr << "Error: " << e.what()
      << std::endl << visibleOptions
      << std::endl << defaults.str();
    exit(1);
  }
}

// ___________________________________________________________________________
void SearchServer::setOptions() {
  // _vocabularyFileName = optionVariables["vocabulary-file"].as<string>();
  if (_optionVariables.count("port"))
    _port = _optionVariables["port"].as<unsigned int>();
  else
    throw po::error("No port given.");
  if (_optionVariables.count("k-gram-length"))
    _k = _optionVariables["k-gram-length"].as<unsigned int>();
  if (_optionVariables.count("results"))
    _numberOfResults = _optionVariables["results"].as<size_t>();
  if (_optionVariables.count("bm25b"))
    _bm25b = _optionVariables["bm25b"].as<float>();
  if (_optionVariables.count("bm25k"))
    _bm25k = _optionVariables["bm25k"].as<float>();
  if (_optionVariables.count("input-file"))
    _file = _optionVariables["input-file"].as<string>();
  else
    throw po::error("No input-file given");
  if (_optionVariables.count("web-root"))
    _webRoot = _optionVariables["web-root"].as<string>();
}

// ___________________________________________________________________________
void SearchServer::run() {
  cout << "Building index of posts ... " << flush << endl;
  _invertedIndex.buildFromCsvFile(_file, _bm25k, _bm25b);
  cout << "Building index of vocabulary ... " << flush << endl;
  _queryProcessor.init(_invertedIndex, _k);
  cout << "Starting up Server-Loop ... " << endl;
  runServer();
}

// ___________________________________________________________________________
void SearchServer::runServer() {
  try {
    // Create socket and bind to and listen on given port.
    boost::asio::io_service io_service;
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), _port));

    // Wait for requests and process them.
    int i = 0;
    while (true) {
      // Wait for request.
      cout << "\x1b[1m\x1b[34m[" << (++i) << "] Waiting for query on port "
        << _port << " ... \x1b[0m" << flush;
      tcp::socket socket(io_service);
      acceptor.accept(socket);
      std::time_t now = std::time(0);
      std::string daytime = std::ctime(&now);  // NOLINT
      cout << "done, received new request on " << daytime << std::flush;

      // Get the request string.
      std::vector<char> requestBuffer(1000);
      boost::system::error_code read_error;
      socket.read_some(boost::asio::buffer(requestBuffer), read_error);
      std::string request(requestBuffer.size(), 0);
      std::copy(requestBuffer.begin(), requestBuffer.end(), request.begin());
      for (size_t i = 0; i < request.size(); i++) {
        request[i] = isspace(request[i]) ? ' ' : request[i];
      }
      cout << "request string is \"" << (request.size() < 99 ? request :
          request.substr(0, 87) + "...") << "\"" << endl;

      string answer;

      // Parse URL
      replaceUrlCodes(&request);
      size_t argPos = request.find("?");
      if (argPos == string::npos) {
        try {
          string path = getFilePath(request);
          cout << "path: " << path << endl;
          answer = http200(path);
        } catch(const Error501& e) {
          cerr << "\x1b[31m" << e.what() << "\x1b[0m" << endl << flush;
          answer = http418(e.what());
        } catch(const Error404& e) {
          cerr << "\x1b[31m" << e.what() << "\x1b[0m" << endl << flush;
          answer = http418(e.what());
        }
      } else {
        std::string query;
        std::ostringstream jsonp;
        size_t numberOfResults = atoi(getValue(request, "number").c_str());
        // Is it a vocabulary-lookup?
        if ((query = getValue(request, "vocabularyLookup")).size()) {
          cout << "vocabularyLookup: query string is \"" << query
            << "\"; number: results requested: " << numberOfResults << endl;
          vector<string> matches =
            _queryProcessor.similarWords(numberOfResults, query);
          // Send a JSONP object containing the answer.
          jsonp << "similarWordsCallback({" << "\"matches\":[";
          for (vector<string>::iterator it = matches.begin();
              it < matches.end();
              ++it) {
            jsonp << "\"" << *it << "\"";
            if (it + 1 != matches.end())
              jsonp << ",";
          }
          jsonp << "]});";
        }
        if ((query = getValue(request, "searchQuery")).size()) {
          cout << "searchQuery: query string is \"" << query << "\"" << endl;
          vector<size_t> recordIds =
            _queryProcessor.searchRecords(numberOfResults, query);
          jsonp << "searchRecordsCallback({" << "\"matches\":[";
          for (vector<size_t>::iterator it = recordIds.begin();
              it < recordIds.end(); ++it) {
            jsonp << "\"" << _invertedIndex.getUrlFromId(*it) << "\"";
            if (it + 1 != recordIds.end())
              jsonp << ",";
          }
          jsonp << " ]});";
        }
        answer = http200(jsonp.str(), "application/javascript");
      }

      boost::system::error_code write_error;
      boost::asio::write(socket, boost::asio::buffer(answer),
          boost::asio::transfer_all(), write_error);
    }
  } catch(const std::exception& e) {
    cerr << e.what() << endl;
  }
}

// ___________________________________________________________________________
string SearchServer::getValue(string const& request, string const& name) const {
  std::stringstream searchTerm;
  searchTerm << name << "=";
  size_t prefixLength = name.size() + 1;
  size_t pos1 = request.find(searchTerm.str());
  size_t pos2 = std::min(request.find("&", pos1), request.find(" HTTP", pos1));
  if (pos1 == string::npos || pos2 == string::npos)
    return "";
  else
    return request.substr(pos1 + prefixLength, pos2 - pos1 - prefixLength);
}

// ___________________________________________________________________________
string SearchServer::getFilePath(string const& request) const {
  size_t pos1 = request.find("GET /") + 4;
  size_t pos2 = std::min(request.find("?", pos1), request.find(" HTTP", pos1));
  if (pos1 - 4 == string::npos || pos2 == string::npos)
    throw Error501("Error parsing request: " + request);
  std::stringstream path;
  path << _webRoot << request.substr(pos1, pos2 - pos1);
  if (path.str()[path.str().size() - 1] == '/') path << "index.html";

  ifstream f(path.str().c_str());
  if (f.is_open())
    return path.str();
  else
    throw Error404(path.str());
}

// ___________________________________________________________________________
size_t SearchServer::maxEditDistance(string const& query) {
  return ceil(query.size() / 5.0);
}

// ___________________________________________________________________________
void SearchServer::replaceUrlCodes(string* request) {
  map<string, string> urlToString = {
    {"%20", " "},
    {"+", " "},
    {"%22", "\""},
    {"%7E", "~"},
    {"%C3%A4", "ä"},
    {"%C3%96", "A"},
    {"%C3%B6", "ö"},
    {"%C3%96", "Ö"},
    {"%C3%BC", "ü"},
    {"%C3%PC", "Ü"}
  };

  for (map<string, string>::const_iterator it = urlToString.begin();
      it != urlToString.end();
      ++it) {
    size_t position = 0;
    while (
        (position = request->find(it->first, position)) &&
        position != std::string::npos) {
      request->replace(
          position, it->first.length(), it->second);
      position += it->second.size();
    }
  }
}


/*
   try {
   while (it < request.end()) {
   while (it < request.end() and *it != '%') result << *it++;
   if (*++it == '2') {
   if (*++it == '0')
   result << ' ';
   else if (*it == '2')
   result << "\"";
   else
   throw Error501("Error: Unvalid URL-Code");
   ++it;
   } else if (*it == '7') {
   if (*++it == 'E')
   result << '~';
   else
   throw Error501("Error: Unvalid URL-Code");
   ++it;
   } else if (*it == 'C') {
   if (*++it != '%')
   throw Error501("Error: Unvalid umlaut in URL.");
   if (*++it == 'A' or *it == 'B') {
   if (*++it == '4')
   result << "ä";
   else if (*it == '6')
   result << "ö";
   else if (*it == 'C')
   result << "ü";
   } else {
   if (*++it != '8' and *it != '9')
   throw Error501("Error: Unvalid capital umlaut in URL.");
   if (*++it == '4')
   result << "Ä";
   else if (*it == '6')
   result << "Ö";
   else if (*it == 'C')
   result << "Ü";
   }
   } else {
   throw Error501("Error: Unvalid URL-Code");
   }
   }
   } catch(const int&) {
   throw Error501("Error replacing URL-Codes.");
   }
   return result.str();
   */
