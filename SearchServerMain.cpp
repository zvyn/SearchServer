// Copyright 2012, Milan Oberkirch
// Author: Milan Oberkirch <oberkirm@informatik.uni-freiburg.de>

#include "./SearchServer.h"

int main(int argc, char** argv) {
  SearchServer server;
  server.parse(argc, argv);
  server.run();
}
