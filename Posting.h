// Copyright 2012, Milan Oberkirch
// Author: Milan Oberkirch <oberkirm@informatik.uni-freiburg.de>

#ifndef POSTING_H_
#define POSTING_H_

#include <string>
#include <stdexcept>

using std::string;

// Class to hold a Pair of document-Id and score, wich can be assigned to a
// string in an inverted index.
class Posting {
 public:
  // Number to identify an document globally
  size_t documentId;
  // Score of the Posting (http://en.wikipedia.org/wiki/Okapi_BM25)
  float score;

  // Constructors
  explicit Posting();
  explicit Posting(int const& documentId, float const& score);

  // Compare by documentId
  bool operator==(const Posting &other) const;
  bool operator==(const size_t &otherDocumentId) const;
  bool operator!=(const Posting &other) const;
  // Order by Irrelevance
  bool operator<(const Posting &other) const;

  // Represent values as string
  string toString() const;
};

#endif  // POSTING_H_
