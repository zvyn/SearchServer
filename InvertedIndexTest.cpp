// Copyright 2012, Milan Oberkirch
// Author: Milan Oberkirch <oberkirm@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <fstream>  // NOLINT
#include <vector>
#include <string>
#include "./InvertedIndex.h"
#include "./Posting.h"

const char mockupFileName[] = "InvertedIndexMockup.test.tmp";
const char mockup2FileName[] = "InvertedIndexMockup2.test.tmp";
InvertedIndex ii;

// ___________________________________________________________________________
TEST(InvertedIndex, createMockup) {
  // Create small mockup, just to visit some code later on
  std::ofstream mockup(mockupFileName);
  if (mockup.is_open()) {
    mockup << "first_url\tsome record about nothing\n"
      << "www.example.com\tthis is About anything\n"
      << "www.example.com\tthis is About anything\n";
  } else {
    std::cerr << "Cannot open mockup-file";
  }

  // Collection of some strings where buildFromCsvFile failed.
  std::ofstream mockup2(mockup2FileName);
  if (mockup2.is_open()) {
    mockup2
      << "url\tcrawford brough macpherson o. c. m. sc.\n"
      << "url\tcrawford brough macpherson o. c. m. sc.\n"
      << "url2\tcugh macpherson o. c. m. sc.\n"
      << "url2\tugh macpherson o. c. m. sc.\n"
      << "http://en.wikipedia.org/wiki/Karl_Rathgen\tKarl Rathgen, Economist.\n"
      << "urlt\tmassimo pigliucci (born jan 16, 1964) at cuny-lehman college.\n"
      << "url\tcrawford brough macpherson o. c. m. sc.\n";
  } else {
    exit(23);
  }
}

// ___________________________________________________________________________
TEST(InvertedIndex, buildFromCsvFile) {
  ii.buildFromCsvFile(mockupFileName);
  EXPECT_EQ(2, ii._urls.size());
  EXPECT_EQ(2, ii._records.size());
  EXPECT_EQ(6, ii._invertedLists.size());
  EXPECT_EQ(4, ii._documentLengthInWords.at(0));
  EXPECT_EQ(2, ii._invertedLists.at("about").size());
  EXPECT_EQ("this is About anything this is About anything", ii._records.at(1));
}

// ___________________________________________________________________________
TEST(InvertedIndex, getRecordFromId) {
  EXPECT_EQ("some record about nothing", ii.getRecordFromId(0));
  EXPECT_EQ("this is About anything this is About anything",
          ii.getRecordFromId(1));
}

TEST(InvertedIndex, buildFromCsvFileLastCharIsSpecial) {
  ii.buildFromCsvFile(mockup2FileName);
}

// ___________________________________________________________________________
TEST(InvertedIndex, clear) {
  ii.clear();
  EXPECT_TRUE(ii._urls.empty());
  EXPECT_TRUE(ii._records.empty());
  EXPECT_TRUE(ii._invertedLists.empty());
  EXPECT_TRUE(ii._documentLengthInWords.empty());
}

// ___________________________________________________________________________
TEST(InvertedIndex, getPostingsFromWord) {
  ii.clear();
  ii._invertedLists["test"].push_back(Posting(5, 0.1));
  EXPECT_EQ(5, ii.getPostingsFromWord("test").at(0).documentId);
  EXPECT_NEAR(0.1, ii.getPostingsFromWord("test").at(0).score, 1e-5);
}

// ___________________________________________________________________________
TEST(InvertedIndex, getUrlFromId) {
  ii.clear();
  ii._urls.push_back("first_url");
  EXPECT_EQ("first_url", ii.getUrlFromId(0));
}

