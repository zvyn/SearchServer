// Copyright 2012, Milan Oberkirch
// Author: Milan Oberkirch <oberkirm@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <fstream>  // NOLINT
#include <vector>
#include <string>
#include "./ApproximateMatching.h"

using std::string;

const char mockupFileName[] = "ApproximateMatching.test.tmp";
ApproximateMatching approximateMatching;
InvertedIndex ii;

// ___________________________________________________________________________
TEST(ApproximateMatching, createMockup) {
  // Create small mockup, just to visit some code later on
  std::ofstream mockup(mockupFileName);
  if (mockup.is_open()) {
    mockup << "first_url\tanalphabet\n"
      << "www.example.com\tvocabulary\n";
  } else {
    std::cerr << "Cannot open mockup-file";
  }
}

// ___________________________________________________________________________
TEST(ApproximateMatching, init) {
  ii.buildFromCsvFile(mockupFileName);
  approximateMatching.init(ii, 5, '+');
  EXPECT_EQ('+', approximateMatching.dummyChar());
  EXPECT_EQ(5, approximateMatching.k());
}

// ___________________________________________________________________________
TEST(ApproximateMatching, mergeInvertedLists) {
  ApproximateMatching approximateMatching;
  vector< vector<size_t> > input = { {1, 3, 4, 6, 9, 11}, {2, 5, 7, 8, 10} };
  vector<size_t> expected = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
  vector<size_t> result = approximateMatching.mergeInvertedLists(input);

  // When I do an assert here, it is legit to only check one iterator in the
  // following loop
  ASSERT_EQ(expected.size(), result.size());
  vector<size_t>::iterator itExpected = expected.begin();
  vector<size_t>::iterator itResult = result.begin();
  for (; itExpected < expected.end(); ++itExpected, ++itResult)
    EXPECT_EQ(*itExpected, *itResult);

  // More then two lists, with dublicats
  input = { {1, 2, 3, 10}, {2, 3, 4}, {1, 4, 6, 8}, {5, 7, 9} };
  expected = {1, 1, 2, 2, 3, 3, 4, 4, 5, 6, 7, 8, 9, 10};
  result = approximateMatching.mergeInvertedLists(input);
  ASSERT_EQ(expected.size(), result.size());
  itExpected = expected.begin();
  itResult = result.begin();
  for (; itExpected < expected.end(); ++itExpected, ++itResult)
    EXPECT_EQ(*itExpected, *itResult);
}

// ___________________________________________________________________________
TEST(ApproximateMatching, computeEditDistance) {
  ApproximateMatching approximateMatching;

  unsigned int result = approximateMatching.
    computeEditDistance("B", "B");
  unsigned int expected = 0;
  EXPECT_EQ(expected, result);

  result = approximateMatching.
    computeEditDistance("BREAD", "BOARD");
  expected = 3;
  EXPECT_EQ(expected, result);

  result = approximateMatching.
    computeEditDistance("ha", "hans");
  expected = 2;
  EXPECT_EQ(expected, result);

  result = approximateMatching.
    computeEditDistance("halten", "hans");
  expected = approximateMatching.
    computeEditDistance("halt", "hans") + 2;
  EXPECT_EQ(expected, result);
}

// ___________________________________________________________________________
TEST(ApproximateMatching, computeApproximateMatches) {
  string input;
  vector<string> expected;
  vector<string> actual;

  input = "vocabluary";
  expected = {"analphabet", "vocabulary"};
  actual = approximateMatching.computeApproximateMatches(
      input, 10);
  ASSERT_EQ(expected.size(), actual.size());
  EXPECT_EQ(expected[0], actual[0]);

  input = "vocabeluary";
  expected = {"vocabulary"};
  actual = approximateMatching.computeApproximateMatches(
      input, 5);
  ASSERT_EQ(expected.size(), actual.size());
  EXPECT_EQ(expected[0], actual[0]);

  input = "vocabluary";
  expected = {"vocabulary"};
  actual = approximateMatching.computeApproximateMatches(
      input, 2);
  ASSERT_EQ(expected.size(), actual.size());
  EXPECT_EQ(expected[0], actual[0]);

  input = "vocabeluary";
  expected = {"analphabet", "vocabulary"};
  actual = approximateMatching.computeApproximateMatches(
      input, 10);
  ASSERT_EQ(expected.size(), actual.size());
  EXPECT_EQ(expected[0], actual[0]);
}
