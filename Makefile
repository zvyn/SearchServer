CXX = g++ -Wall -std=c++0x
HAEDER = $(wildcard *.h)
OBJECTS = $(addsuffix .o, $(basename $(filter-out %Main.cpp %Test.cpp,$(wildcard *.cpp))))
MAIN_BINARIES = $(basename $(wildcard *Main.cpp))
TEST_BINARIES = $(basename $(wildcard *Test.cpp))

all: checkstyle compile test

build: $(MAIN_BINARIES) $(TEST_BINARIES)

compile: CXX += -O3 -DNDEBUG
compile: build

static: CXX += -static
static: compile

debug: CXX += -g
debug: build

test: $(TEST_BINARIES) 
	for T in $(TEST_BINARIES); do ./$$T; done
	rm -f *.test.tmp core

checkstyle:
	cpplint *.cpp *.h || ./cpplint.py
clean:
	rm -f *.o
	rm -f *Main
	rm -f *Test
	rm -f *.test.tmp
	
%Main: %Main.o $(OBJECTS)
	$(CXX) -o $@ $^ -lpthread -lboost_system -lboost_program_options

%Test: %Test.o $(OBJECTS)
	$(CXX) -o $@ $^ -lgtest -lgtest_main -lboost_system -lpthread -lboost_program_options


%Test.o: %Test.cpp $(HAEDER)
	$(CXX) -c $< -lgtest -lgtest_main -lpthread

%Main.o: %Main.cpp $(HAEDER)
	$(CXX) -c $<

verbose: 
	# CXX=$(CXX)
	# HAEDER=$(HAEDER)
	# OBJECTS=$(OBJECTS)
	# MAIN_BINARIES=$(MAIN_BINARIES)
	# TEST_BINARIES=$(TEST_BINARIES)

	
