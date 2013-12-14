// Copyright 2012, Milan Oberkirch
// Author: Milan Oberkirch <oberkirm@informatik.uni-freiburg.de>

#ifndef QUERYPROCESSOR_H_
#define QUERYPROCESSOR_H_

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <map>
#include "./InvertedIndex.h"
#include "./ApproximateMatching.h"
#include "./Posting.h"

using std::map;
using std::string;
using std::vector;

// Class for processing queries with two keywords based on an inverted index.
class QueryProcessor {
  InvertedIndex const *_index;
  ApproximateMatching _approximateMatching;

 public:
  // Initialice vovabulary in _approximateMatching and set index for search.
  void init(InvertedIndex const& index, int const& k);
  // Answer given query. Return list of matching record ids.
  vector<size_t> searchRecords(size_t numberOfResults, string query);
  // Lookup words with similar prefix.
  vector<string> similarWords(size_t numberOfResults, string const& query);

 private:
  // Intersect two inverted lists and return the result list.
  FRIEND_TEST(QueryProcessor, intersect);
  FRIEND_TEST(QueryProcessor, intersectFromEx03);
  vector<Posting> intersect(vector<Posting> list1,
      vector<Posting> list2);
};

#endif  // QUERYPROCESSOR_H_
