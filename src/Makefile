# ============================================================================
#  mlux/src/Makefile — builds the main mlux binary
# ============================================================================

rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# --- Resolve sources --------------------------------------------------------
SOURCES := $(patsubst ./%,%,$(call rwildcard,.,*.cpp))
OBJECTS := $(SOURCES:%.cpp=$(BUILD_OUT)/%.o)
DEPS    := $(OBJECTS:.o=.d)

# --- Target -----------------------------------------------------------------
TARGET := $(BUILD_OUT)/$(PROJECT)

.PHONY: all clean

all: $(TARGET)

# --- Link -------------------------------------------------------------------
$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@

# --- Compile (auto-dependency generation) -----------------------------------
$(BUILD_OUT)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# --- Include auto-generated dependency files ---------------------------------
-include $(DEPS)

clean:
	rm -rf $(BUILD_OUT)/src
