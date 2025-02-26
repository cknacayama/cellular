SRC_PATH := src
OBJ_PATH := obj
TARGET_PATH := bin
INCLUDE_PATH := include

CC := g++-13
CPP_STD := -std=c++23
CPP_FLAGS := -O3
CPP_FLAGS += -I$(INCLUDE_PATH) $(CPP_STD) -g -Werror -Wall -Wextra -Wno-unused-parameter
LIB_FLAGS := -lGL -lglfw
OBJ_FLAGS := $(CPP_FLAGS) -c

TARGET_NAME := cellular
TARGET := $(TARGET_PATH)/$(TARGET_NAME)
SRC := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.c*)))
OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))

CLEAN_LIST := $(TARGET) \
			  $(OBJ) \
			  $(TARGET_NAME).zip

default: makedir all

$(TARGET): $(OBJ)
	$(CC) -o $@ $(OBJ) $(CPP_FLAGS) $(LIB_FLAGS) 

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c*
	$(CC) $(OBJ_FLAGS) -o $@ $<

.PHONY: makedir
	@mkdir -p $(TARGET_PATH) $(OBJ_PATH)

.PHONY: all
all: $(TARGET)

.PHONY: clean
run: $(TARGET)
	@./$(TARGET)

.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(CLEAN_LIST)

.PHONY: zip
zip:
	@make clean
	@echo ZIP
	@zip $(TARGET_NAME).zip src/ include/ obj/ bin/ Makefile -r
