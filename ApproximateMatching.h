// Copyright 2012, Milan Oberkirch
// Author: Milan Oberkirch <oberkirm@informatik.uni-freiburg.de>

#ifndef APPROXIMATEMATCHING_H_
#define APPROXIMATEMATCHING_H_

#include <gtest/gtest.h>
#include <map>
#include <string>
#include <vector>
#include <utility>
#include "./InvertedIndex.h"

using std::map;
using std::string;
using std::vector;
using std::pair;

class ApproximateMatching {
  // See http://en.wikipedia.org/wiki/n-gram (with  n == k)
  unsigned int _kGramLength;
  // Stores words by implicitly mapping them to ids (as positions)
  vector<string> _words;
  // Maps k-grams to their word-ids.
  map<string, vector<size_t> > _invertedLists;
  // The char used to fill up length of grams whichi would have less then k
  // chars.
  char _dummyChar;

  // Mechurements
  clock_t _indexCreationTime;
  vector<clock_t> _editDistanceTimes;
  vector<clock_t> _queryComputationTimes;
  vector<size_t> _matchesCounts;

 public:
  // Getter to previously described members.
  /** EVIL!
  const char *dummyChar = &_dummyChar;
  const unsigned int *k = &_kGramLength;
  const vector<string> *words = &_words;
  const map<string, vector<size_t> > *invertedLists = &_invertedLists;
  */
  const char& dummyChar() const { return _dummyChar; }
  const unsigned int k() const { return _kGramLength; }
  const vector<string>& words() const { return _words; }
  const map<string, vector<size_t> >& invertedLists() const {
    return _invertedLists;
  }

  // Set the member-variables, needed for buildIndex
  void init(InvertedIndex const& index, unsigned int const& k,
      char const& dummyChar = '+');

  // Returns all words within the given maxEditDistance from the proviously
  // build index.
  vector<string> computeApproximateMatches(
      string const& word, unsigned int const& maxEditDistance,
      const int& numberOfResults = 10) const;

  // Prints the invertedLists
  void printInvertedLists();

 private:
  bool pairsAreInRightOrder(
    pair<size_t, string> const& p1, pair<size_t, string> const& p2);
  // Build a k-gram index for the vocabluary.
  void buildIndex(InvertedIndex const& invertedIndex);
  // See http://en.wikipedia.org/wiki/Edit_distance.
  // Needs O(|word1| * |word2|).
  FRIEND_TEST(ApproximateMatching, computeEditDistance);
  unsigned int computeEditDistance(
      string const& word1, string const& word2, bool prefix = false) const;

  // Returns the ids of the union of invertedLists.
  FRIEND_TEST(ApproximateMatching, mergeInvertedLists);
  vector<size_t> mergeInvertedLists(
      vector< vector<size_t> > invertedLists) const;
};

#endif  // APPROXIMATEMATCHING_H_
