CC := gcc
AR := ar
ARFLAGS := rcs
LDFLAGS :=

# Directories
SRC_DIR := src
INC_DIR := -Iinc -Icommon
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
DEP_DIR := $(BUILD_DIR)/dep
LIB_DIR := $(BUILD_DIR)/lib
TEST_DIR := test

# Find all the C source files
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS := $(SRCS:$(SRC_DIR)/%.c=$(DEP_DIR)/%.d)
LIB := $(SRCS:$(SRC_DIR)/%.c=$(DEP_DIR)/%.d)

CFLAGS := $(INC_DIR) 

# Define the application executable name
APP_NAME := hmm
LIB_NAME := libhmm.a

# Default target
all: $(APP_NAME)
	@echo "Build successful"

# Link the application
$(APP_NAME): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile the source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(DEP_DIR)
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MMD -MF $(DEP_DIR)/$*.d

# Create the dependency directory
$(DEP_DIR):
	mkdir -p $@

# Include the dependency files
-include $(DEPS)

# Static library target
.PHONY: static_lib
static_lib: $(OBJS)
	mkdir -p $(LIB_DIR)
	$(AR) $(ARFLAGS) $(LIB_DIR)/$(LIB_NAME) $^
	@echo "Static library build successful"

# Test target
.PHONY: test
test: $(OBJS)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BUILD_DIR)/hmm_test $(TEST_DIR)/*.c $^
	@echo "Build successful"
	
# Clean target
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)