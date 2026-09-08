CXX      := clang++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -Isrc

SRC      := src
BUILD    := build
TARGET   := $(BUILD)/mlux

rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SOURCES  := $(call rwildcard,$(SRC),*.cpp)
OBJECTS  := $(SOURCES:$(SRC)/%.cpp=$(BUILD)/%.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $^ -o $@

$(BUILD)/%.o: $(SRC)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)
