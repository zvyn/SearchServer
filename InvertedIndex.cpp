// Copyright 2012, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Authors: Hannah Bast <bast@informatik.uni-freiburg.de>,
//          Milan Oberkirch <oberkirm@informatik.uni-freiburg.de>

#include "./InvertedIndex.h"
#include <assert.h>
#include <cmath>
#include <fstream>  // NOLINT
#include <map>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include "./Posting.h"

using std::ifstream;
using std::map;
using std::string;
using std::vector;

const size_t minWordLength = 2;
// Randomly choosen upper bound to ensure termination.
const size_t maxRecordLength = 100000;
// Maximum URL-Length for Sitemap-Protocol.
const size_t maxUrlLength = 2047;

// _____________________________________________________________________________
InvertedIndex::InvertedIndex() {
}

// _____________________________________________________________________________
void InvertedIndex::clear() {
  _invertedLists.clear();
  _records.clear();
  _urls.clear();
  _documentLengthInWords.clear();
}

// _____________________________________________________________________________
void InvertedIndex::buildFromCsvFile(string const& fileName,
    float const& bm25k, float const& bm25b) {
  clear();
  ifstream file(fileName.c_str());
  string line;
  size_t pos = 0;
  size_t documentId;

  while (true) {
    getline(file, line);
    if (file.eof()) break;
    // Extract record and url from line
    pos = line.find('\t', 0);
    if (pos > maxUrlLength) {
      throw std::runtime_error("Wrong Format");
    }
    string url = line.substr(0, pos);
    string record = line.substr(pos + 1, maxRecordLength);
    // Parsing one line
    documentId = setIdAndSaveUrlAndRecord(url, record);
    std::transform(record.begin(), record.end(), record.begin(), ::tolower);
    parseRecord(documentId, record);
  }

  calculateScores(bm25k, bm25b);
}

// _____________________________________________________________________________
int InvertedIndex::setIdAndSaveUrlAndRecord(
    string const& url, string const& record) {
  size_t documentId;
  if (_urls.empty() || url.compare(_urls.back()) != 0) {
    _records.push_back(record);
    _urls.push_back(url);
    _documentLengthInWords.push_back(0);
    documentId = _urls.size() - 1;
  } else {
    documentId = _urls.size() - 1;
    // -1 as size_t is the largest value possible for size_t
    assert(static_cast<size_t>(-1) > documentId);
    assert(_records.size() > documentId);
    _records[documentId] = _records[documentId] + " " + record;
  }
  return documentId;
}

// _____________________________________________________________________________
void InvertedIndex::parseRecord(
    size_t const& documentId,
    string const& line) {
  size_t pos = 0;
  string word;

  assert(documentId == _urls.size() - 1);
  while (pos < line.size()) {
    while (pos < line.size() && !isalpha(line[pos])) pos++;
    size_t wordStart = pos;
    while (pos < line.size() && isalpha(line[pos])) pos++;
    size_t wordEnd = pos;
    assert(wordEnd <= line.size());
    assert(wordStart >= 0);

    if (wordEnd > wordStart + minWordLength) {
      assert(_documentLengthInWords.size() > documentId);
      _documentLengthInWords[documentId]++;
      assert(wordEnd - wordStart > 0);
      word = line.substr(wordStart, wordEnd - wordStart);
      assert(word.size() > 0);
      vector<Posting>* current = &_invertedLists[word];
      vector<Posting>::iterator itPosting =
        std::find(current->begin(), current->end(), documentId);
      if (itPosting == current->end()) {
        current->resize(current->size() + 1);
        current->at(current->size() - 1) = Posting(documentId, 1);
      } else {
        itPosting->score = itPosting->score + 1;
      }
    }
  }
}

// _____________________________________________________________________________
void InvertedIndex::printInvertedIndex() const {
  // Print Index
  for (map<string, vector<Posting> >::
      const_iterator it = _invertedLists.begin();
      it != _invertedLists.end(); ++it) {
    std::cout << it->first << "\t";
    vector<Posting> postings = it->second;
    if (!postings.empty())
      std::cout << postings[0].toString() << ";";
    for (size_t i = 1; i < postings.size(); ++i) {
      std::cout << " " << postings[i].toString() << ";";
    }
    std::cout << std::endl;
  }

  std::cout << "\t" << std::endl;
  // Print Urls and Records
  for (size_t documentId = 0; documentId < _urls.size(); ++documentId)
    std::cout << _urls[documentId] << '\t' << _records[documentId] << std::endl;
}

// _____________________________________________________________________________
void InvertedIndex::calculateScores(
    float const& bm25k = 1.75, float const& bm25b = 0.75) {
  size_t avdl = 0;
  for (size_t count = 1; count <= _documentLengthInWords.size(); ++count) {
    avdl = ((count * avdl) + _documentLengthInWords[count - 1]) / (count + 1);
  }
  for (map<string, vector<Posting> >::iterator it = _invertedLists.begin();
      it != _invertedLists.end(); ++it) {
    vector<Posting> *postings = &(it->second);
    size_t df = postings->size();
    for (size_t i = 0; i < postings->size(); ++i) {
      size_t tf = postings->at(i).score;
      size_t dl = _documentLengthInWords[postings->at(i).documentId];
      float tfStar = tf * (bm25k + 1) /
        (bm25k * (1 - bm25b + ((bm25b * dl) / avdl)) + tf);
      float bm25score = tfStar * log2(static_cast<float>(_urls.size()) / df);
      postings->at(i).score = bm25score;
    }
  }
}

// _____________________________________________________________________________
size_t  InvertedIndex::countOfDocumentsContainingWord(
    string const& word) const {
  try {
    return _invertedLists.at(word).size();
  } catch(const std::out_of_range &e) {
    return 0;
  }
}

// _____________________________________________________________________________
vector<Posting> InvertedIndex::getPostingsFromWord(string const& word) const {
  try {
    return _invertedLists.at(word);
  } catch(const std::out_of_range &e) {
    return vector<Posting>();
  }
}

// _____________________________________________________________________________
string InvertedIndex::getUrlFromId(int const& id) const {
  return _urls.at(id);
}

// _____________________________________________________________________________
string InvertedIndex::getRecordFromId(int const& id) const {
  return _records.at(id);
}
