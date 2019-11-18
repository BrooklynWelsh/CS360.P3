CXX = g++

CXXFLAGS = -g -Wall -pthread

all: cheesecloth numbergen

cheesecloth: cheesecloth.o
	$(CXX) $(CXXFLAGS) -o cheesecloth cheesecloth.o

cheesecloth.o:

numbergen: numbergen.o
	$(CXX) $(CXXFLAGS) -o numbergen numbergen.o

numbergen.o:

clean:
	rm -f cheesecloth *.o *.gch
