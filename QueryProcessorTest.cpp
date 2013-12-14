// Copyright 2012, Milan Oberkirch
// Author: Milan Oberkirch <oberkirm@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <vector>
#include <string>
#include "./QueryProcessor.h"
#include "./Posting.h"

using std::string;
QueryProcessor qp;
vector<Posting> list1;
vector<Posting> list2;
vector<Posting> result;

// ___________________________________________________________________________
TEST(QueryProcessor, intersect) {
  list1 = {Posting(2, 0.5), Posting(5, 0.3)};
  list2 = {Posting(2, 0.8), Posting(3, 0.9)};
  result = qp.intersect(list1, list2);
  EXPECT_EQ(1, result.size());
  ASSERT_NEAR(0.4, result.at(0).score, 1e-5);
  ASSERT_EQ(2, result.at(0).documentId);
}

// ___________________________________________________________________________
// Failing test by tutor(js174) adapted to QueryProcessor by mo54
TEST(QueryProcessor, intersectFromEx03) {
  vector<Posting> A = {Posting(1, 1), Posting(4, 1), Posting(5, 1)};
  vector<Posting> B =
  {Posting(1, 1), Posting(2, 1), Posting(3, 1), Posting(49, 1)};
  QueryProcessor q;
  list1 = A;
  list2 = B;
  result = q.intersect(list1, list2);
  vector<Posting> expected = {Posting(1, 1)};
  EXPECT_EQ(expected, result);
}
