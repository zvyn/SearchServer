// Copyright 2012, Milan Oberkirch
// Author: Milan Oberkirch <oberkirm@informatik.uni-freiburg.de>

#include "./QueryProcessor.h"
#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <stdexcept>
#include "./InvertedIndex.h"
#include "./ApproximateMatching.h"
#include "./Posting.h"

using std::map;
using std::string;
using std::vector;

// ___________________________________________________________________________
void QueryProcessor::init(InvertedIndex const& index, int const& k) {
  _index = &index;
  _approximateMatching.init(index, k);
}

// ___________________________________________________________________________
vector<string> QueryProcessor::similarWords(size_t numberOfResults,
    string const& query) {
  vector<string> queryVector;
  split(queryVector, query, boost::is_any_of(" ,+,,"));
  string word = queryVector.back();
  std::transform(word.begin(), word.end(), word.begin(), ::tolower);
  string prefix =
    query.substr(0, query.size() - (queryVector.size() - 1 + word.size()));
  if (!prefix.empty()) prefix += " ";
  vector<string> results =  _approximateMatching.
    computeApproximateMatches(word, (word.size() - 1) / 3, numberOfResults);
  for (vector<string>::iterator it = results.begin();
      it < results.end(); ++it)
    *it = prefix + *it;
  return results;
}

// ___________________________________________________________________________
vector<size_t> QueryProcessor::searchRecords(size_t numberOfResults,
    string query) {
  vector<string> queryVector;
  vector<Posting> postings;
  vector<size_t> result;

  // uniform query
  std::transform(query.begin(), query.end(), query.begin(), ::tolower);
  split(queryVector, query, boost::is_any_of(" ,+,,"));

  // collect candidates
  postings = _index->getPostingsFromWord(queryVector[0]);
  if (postings.empty())
    return vector<size_t>();
  for (size_t i = 1; i < queryVector.size(); ++i) {
    postings = intersect(_index->getPostingsFromWord(queryVector[i]),
        postings);
  }

  // sort the results
  std::partial_sort(
      postings.begin(),
      std::min(postings.end(), postings.begin() + numberOfResults),
      postings.end());

  // convert the result to vector<int>
  for (size_t i = 0; i < numberOfResults && i < postings.size(); i++) {
    result.push_back(postings[i].documentId);
  }

  return result;
}

// ___________________________________________________________________________
vector<Posting> QueryProcessor::intersect(
    vector<Posting> list1, vector<Posting> list2) {
  vector<Posting> result;

  vector<Posting>::const_iterator i = list1.begin();
  vector<Posting>::const_iterator j = list2.begin();

  while (i < list1.end() && j < list2.end()) {
    while (j < list2.end() && i->documentId > j->documentId) ++j;
    if (j == list2.end()) break;
    while (i < list1.end() && j->documentId > i->documentId) ++i;
    if (i == list1.end()) break;
    if (*i == *j) {
      result.push_back(Posting(i->documentId, i->score * j->score));
      ++i;
      ++j;
    }
  }
  return result;
}
