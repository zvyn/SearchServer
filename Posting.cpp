// Copyright 2012, Milan Oberkirch
// Author: Milan Oberkirch <oberkirm@informatik.uni-freiburg.de>

#include "./Posting.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>

using std::string;
using std::vector;
using boost::lexical_cast;

// _____________________________________________________________________________
Posting::Posting() {
  this->score = 0;
  this->documentId = 0;
}

Posting::Posting(int const& documentId, float const& score) {
  this->documentId = documentId;
  this->score = score;
}

// _____________________________________________________________________________
bool Posting::operator==(const Posting &other) const {
  return documentId == other.documentId;
}

bool Posting::operator==(const size_t &otherDocumentId) const {
  return documentId == otherDocumentId;
}

bool Posting::operator!=(const Posting &other) const {
  return documentId != other.documentId;
}

bool Posting::operator<(const Posting &other) const {
  return score > other.score;
}

// _____________________________________________________________________________
string Posting::toString() const {
  return lexical_cast<string>(documentId) + ":" + lexical_cast<string>(score);
}

