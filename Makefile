CXX      := clang++
INCLUDES := $(shell find include antlr -type d)
CXXFLAGS := -std=c++17 -O2  $(addprefix -I,$(INCLUDES)) -I./extlibs
LDFLAGS  := -L./extlibs -lantlr4-runtime -Wl,-rpath=./extlibs
SOURCES  := $(shell find src antlr -name '*.cpp')
TARGET   := compiler

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)