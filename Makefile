SRC_PATH := src
PERF_PATH := perf
OBJ_PATH := obj
TARGET_PATH := bin
INCLUDE_PATH := include
USER_HEADER_PATH := $(INCLUDE_PATH)/cell

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
PERF := $(foreach x, $(PERF_PATH), $(wildcard $(addprefix $(x)/*,.data*)))

USER_HEADERS := $(foreach x, $(USER_HEADER_PATH), $(wildcard $(addprefix $(x)/*,.h*)))
CHECK_LIST := $(filter-out $(SRC_PATH)/gl.cpp,$(SRC))
CHECK_LIST += $(USER_HEADERS)
CLEAN_LIST := $(TARGET) \
			  $(OBJ) \
			  $(PERF) \
			  $(TARGET_NAME).zip \


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

.PHONY: check
check:
	@echo CHECK $(CHECK_LIST)
	@clang-tidy-21 --fix -p . $(CHECK_LIST)

.PHONY: fmt
fmt:
	@echo FORMAT $(CHECK_LIST)
	@clang-format-21 -i $(CHECK_LIST)

.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(CLEAN_LIST)

.PHONY: zip
zip:
	@make clean
	@echo ZIP
	@zip $(TARGET_NAME).zip src/ include/ obj/ bin/ Makefile -r
