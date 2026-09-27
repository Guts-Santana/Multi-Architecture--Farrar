CXX = g++

CXXFLAGS = -std=c++17 -Wall -O3 -mavx2 -Iinclude

SRC = src
BIN = bin
INC = include

TARGET = farrar

all: $(TARGET)

$(BIN):
	mkdir -p $(BIN)

$(TARGET): main.cpp $(BIN)/farrar.o
	$(CXX) $(CXXFLAGS) main.cpp $(BIN)/farrar.o -o $(TARGET)

$(BIN)/farrar.o: $(SRC)/Farrar.cpp \
                 $(INC)/Farrar.hpp \
                 $(INC)/constants.hpp \
                 $(INC)/Buffer.hpp \
                 $(INC)/AVX/AvxTraits.hpp \
                 $(INC)/AVX/AvxOps.hpp
	$(CXX) $(CXXFLAGS) -c $(SRC)/Farrar.cpp -o $(BIN)/farrar.o

clean:
	rm -rf $(BIN)/*.o $(TARGET)