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

M = ""
push:
	git status
	echo "----- WILL BE COMMITING THIS -----"
	sleep 1
	git add .
	git commit -m "$(M)"
	git push origin main



