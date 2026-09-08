CC := g++

SRC := src
INC := include
BUILD : build

TARGET := $(BUILD)/mlux

SOURCES := $(wildcard $(SRC)/*.c)
OBJECTS := $(SOURCES: $(SRC)/%.c=$(BUILD)/%.o)
CFLAGS := -Wall -Wextra -I$(INC)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@

$(BUILD)/%.o: $(SRC)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)

