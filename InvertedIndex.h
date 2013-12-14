// Copyright 2012, Milan Oberkirch
// Author: Milan Oberkirch <oberkirm@informatik.uni-freiburg.de>

#ifndef INVERTEDINDEX_H_
#define INVERTEDINDEX_H_

#include <gtest/gtest.h>
#include <map>
#include <string>
#include <vector>
#include "./Posting.h"

using std::map;
using std::string;
using std::vector;

// Class implementing an inverted index (INV).
class InvertedIndex {
  // list of record ids for each word in the collection.
  map<string, vector<Posting> > _invertedLists;
  vector<string> _records;
  vector<string> _urls;
  vector<size_t> _documentLengthInWords;
  // Tests:
  FRIEND_TEST(InvertedIndex, buildFromCsvFile);
  FRIEND_TEST(InvertedIndex, clear);
  FRIEND_TEST(InvertedIndex, getPostingsFromWord);
  FRIEND_TEST(InvertedIndex, getUrlFromId);

 public:
  InvertedIndex();
  // Getter for the  _invertedLists-map
  /** EVIL
  map<string, vector<Posting> >* invertedLists = &_invertedLists;
  */
  const map<string, vector<Posting> >& invertedLists() const {
    return _invertedLists;
  }
  // Create index from a text collection in CSV format (one record per line,
  // two columns, column 1 = URL, column 2 = text).
  void buildFromCsvFile(
      string const& fileName,
      float const& bm25k = 0.75, float const& bm25b = 1.75);

  // Write inverted index to file
  void printInvertedIndex() const;

  // Get a list of Record-Ids containing a given word.
  vector<Posting> getPostingsFromWord(string const& word) const;

  // Get URL to a given Record-Id
  string getUrlFromId(int const& id) const;

  // Get Record-Text to a given Record-Id
  string getRecordFromId(int const& id) const;

 private:
  // Parse a record and add each word to the respective index list.
  void parseRecord(
      size_t const& documentId, string const& record);
  // Safe the URL and record to the respective vectors.
  // Returns true if the URL was already in the list.
  int setIdAndSaveUrlAndRecord(string const& url, string const& record);
  // Clear inverted list
  void clear();
  // Count of Documents containing a word
  size_t countOfDocumentsContainingWord(string const& word) const;
  // Calculate and set scores in the Postings in _invertedLists
  void calculateScores(float const& bm25k, float const& bm25b);
};

#endif  // INVERTEDINDEX_H_
