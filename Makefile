CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

SRCS = src/main.cpp src/lexer.cpp src/token.cpp src/symbol_table.cpp src/parser_rd.cpp src/tables.cpp src/parser_ll.cpp src/parser_lr.cpp
OBJS = $(SRCS:.cpp=.o)

TARGET = minicompiler

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET) $(TARGET).exe
