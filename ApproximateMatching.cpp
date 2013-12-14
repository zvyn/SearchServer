// Copyright 2012, Milan Oberkirch
// Author: Milan Oberkirch <oberkirm@informatik.uni-freiburg.de>

#include "./ApproximateMatching.h"
#include <boost/bind.hpp>
#include <cstdlib>
#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include <utility>
#include "./InvertedIndex.h"

using std::map;
using std::pair;
using std::string;
using std::vector;
using std::min;
using std::max;

// ............................................................................
void ApproximateMatching::init(
    InvertedIndex const& index, unsigned int const& k, char const& dummyChar) {
  _kGramLength = k;
  _dummyChar = dummyChar;
  buildIndex(index);
}

// ............................................................................
void ApproximateMatching::
buildIndex(InvertedIndex const& invertedIndex) {
  clock_t start = clock();
  // This is expensive but saves some time in more frequently called methods
  vector<pair<size_t, string> >wordFrequencies;
  for (map<string, vector<Posting> >::const_iterator
      it = invertedIndex.invertedLists().begin();
      it != invertedIndex.invertedLists().end(); ++it) {
    wordFrequencies.push_back(
        pair<size_t, string>(it->second.size(), it->first));
  }
  // sort by document frequencies
  std::sort(wordFrequencies.begin(), wordFrequencies.end(),
      boost::bind(&std::pair<size_t, string>::first, _1) >
      boost::bind(&std::pair<size_t, string>::first, _2));
  // put it into _words and _invertedLists
  string word;
  unsigned int position;
  size_t wordId = 0;
  string padding(_kGramLength - 1, _dummyChar);
  for (vector<pair<size_t, string> >::iterator it = wordFrequencies.begin();
      it != wordFrequencies.end(); ++it) {
    _words.push_back(it->second);
    position = 0;
    word = padding + (it->second);
    while (position + _kGramLength -1 < word.size())
      _invertedLists[word.substr(position++, _kGramLength)].push_back(wordId);
    ++wordId;
  }
  _indexCreationTime = (clock() - start);
}

// ............................................................................
void ApproximateMatching::
printInvertedLists() {
  for (map<string, vector<size_t> >::iterator mapIt = _invertedLists.begin();
      mapIt != _invertedLists.end(); ++mapIt) {
    std::cout << mapIt->first << "\t";
    for (vector<size_t>::iterator vectorIt = mapIt->second.begin();
        vectorIt != mapIt->second.end(); ++vectorIt)
      std::cout << *vectorIt << " ";
    std::cout << std::endl;
  }
}

// ............................................................................
vector<string> ApproximateMatching::
computeApproximateMatches(string const& word,
    unsigned int const& maxEditDistance, const int& numberOfResults) const {
  vector<size_t> candidates;
  vector<size_t> newCandidates;
  size_t id;
  vector<string> result;

  // If the Edit-Distance is allowed to be bigger than the input-length,
  // the whole vocabulary matches
  if (word.size() < maxEditDistance + (_kGramLength - 1)) {
    for (vector<string>::const_iterator it = _words.begin();
        it < min(_words.end(), _words.begin() + numberOfResults); ++it)
      result.push_back(*it);
    return result;
  }

  // merge lists of the k-grams of word
  string padding(_kGramLength - 1, _dummyChar);
  string paddedWord = padding + word;
  vector< vector<size_t> > lists;

  for (unsigned int pos = 0; pos + _kGramLength - 1 < paddedWord.size(); ++pos)
    if (_invertedLists.find(
          paddedWord.substr(pos, _kGramLength)) != _invertedLists.end())
      lists.push_back(_invertedLists.at(paddedWord.substr(pos, _kGramLength)));
  candidates = mergeInvertedLists(lists);

  // Find all candidates for which the edit distance is at most the
  // maxEditDistance.
  int sub = _kGramLength * maxEditDistance;
  for (vector<size_t>::iterator it = candidates.begin();
      it < candidates.end() && it >= candidates.begin(); ++it) {
    id = *it;
    int jump = max<int>(0, word.size() - sub);
    if ((it + jump < candidates.end() && *(it + jump) == id) &&
        (newCandidates.empty() || newCandidates.back() != id))
      newCandidates.push_back(id);
  }

  // Pusch all remaining newCandidates with
  // Edit-Distance < maxEditDistance into result
  for (vector<size_t>::iterator it = newCandidates.begin();
      it < min(newCandidates.end(), newCandidates.begin() + numberOfResults);
      ++it)
    if (computeEditDistance(word, _words[*it], true) < maxEditDistance + 1)
      result.push_back(_words[*it]);
  /* for debugging:
     std::cout << "candidates are: { ";
     for (vector<size_t>::iterator it = candidates.begin();
     it < candidates.end(); ++it)
     std::cout << *it << " ";
     std::cout << "}\n";

     std::cout << "newCandidates are: { ";
     for (vector<size_t>::iterator it = newCandidates.begin();
     it < newCandidates.end(); ++it)
     std::cout << *it << " ";
     std::cout << "}\n";

     std::cout << "results are: { ";
     for (vector<string>::iterator it = result.begin();
     it < result.end(); ++it)
     std::cout << *it << " ";
     std::cout << "}\n";
     */

  return result;
}

// ............................................................................
unsigned int ApproximateMatching::computeEditDistance(
    string const& word1, string const& word2, bool prefix) const {
  size_t ped = -1;
  size_t m[word1.length() + 1][word2.length() + 1];
  for (size_t i = 0; i < word1.length() + 1; ++i) {
    m[i][0] = i;
  }
  for (size_t j = 0; j < word2.length() + 1; ++j) {
    m[0][j] = j;
  }
  for (size_t i = 1; i < word1.length() + 1; ++i) {
    for (size_t j = 1; j < word2.length() + 1; ++j) {
      m[i][j] =
        min(
            min(
              m[i - 1][j - 1]
              + (word1[i - 1] == word2
                [j - 1] ? 0 : 1), m[i - 1][j] + 1),
            m[i][j - 1] + 1);
      ped = min(ped, m[word1.length()][j]);
    }
  }
  return prefix ? ped : m[word1.length()][word2.length()];
}

// ............................................................................
vector<size_t> ApproximateMatching::mergeInvertedLists(
    vector< vector<size_t> > invertedLists) const {
  vector<size_t> result;

  if (invertedLists.empty())
    return result;

  if (invertedLists.size() == 2) {
    result.resize(invertedLists[0].size() +  invertedLists[1].size());
    std::merge(invertedLists[0].begin(), invertedLists[0].end(),
        invertedLists[1].begin(), invertedLists[1].end(), result.begin());
    return result;
  }

  vector<size_t> list1 = invertedLists.back();
  invertedLists.pop_back();
  vector<size_t> list2 = mergeInvertedLists(invertedLists);
  result.resize(list1.size() + list2.size());

  std::merge(
      list1.begin(), list1.end(), list2.begin(), list2.end(), result.begin());

  return result;
}

/* It sayed "union(!)" (yes, with the '!') :(
   vector< vector<size_t> >::iterator invertedListsIt = invertedLists.begin();
   vector<size_t> result(*invertedListsIt++);

   for (;invertedListsIt < invertedLists.end(); ++invertedListsIt) {
   vector<size_t>::iterator i = result.begin();
   vector<size_t>::iterator j = (*invertedListsIt).begin();
   vector<size_t> newResult;
   while (i < result.end() && j < (*invertedListsIt).end()) {
   while (j < (*invertedListsIt).end() && *i > *j) newResult.push_back(*j++);
   if (j == (*invertedListsIt).end()) {
   for (;i < result.end(); ++i)
   newResult.push_back(*i);
   break;
   }
   while (i < result.end() && *j > *i) newResult.push_back(*i++);
   if (i == (*invertedListsIt).end()) break;
   if (i == result.end()) {
   for (;j < (*invertedListsIt).end(); ++j)
   newResult.push_back(*j);
   break;
   }
   if (*i == *j) {
   newResult.push_back(*i++);
   ++j;
   }
   }
   result = newResult;
   }
   return result;
   } */
