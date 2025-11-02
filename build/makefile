.PHONY: all clean

### Target Platform ####################
TARGET := zxn

### Project Name #######################
APPNAME := bindump

### Binary type ########################
APPTYPE := dotn

### OS specific settings ###############
ifeq ($(OS),Windows_NT)
RM := rm -f
MV := mv -f
else
RM := rm -f
MV := mv -f
endif

### Build Type #########################
BUILD ?= release

### Source Directories #################
LIB_DIR = ../lib
SRC_DIR = ../src
INC_DIR = ../inc
BUILD_DIR = .

### Source Files #######################
SRCS  = $(SRC_DIR)/main.c
SRCS += $(SRC_DIR)/bindump.c
SRCS += $(LIB_DIR)/libzxn/src/libzxn.c
SRCS += $(LIB_DIR)/libzxn/src/libzxn.asm

### Compiler Flags #####################
CFLAGS = -compiler=sdcc --vc -clib=sdcc_iy -SO3 --opt-code-size -I$(INC_DIR) -I$(LIB_DIR)/libzxn/inc -pragma-include:$(INC_DIR)/zpragma.inc

# Select CRT
CFLAGS += -startup=30

# tradeoff speed vs. code quality
CFLAGS += --max-allocs-per-node200000

ifeq ($(BUILD), debug)
# verbose output
CFLAGS += -v

# create list files
CFLAGS += --list

# create debug code
CFLAGS += -D__DEBUG__
endif

### Linker Flags #######################
LDFLAGS = -subtype=$(APPTYPE) -Cz"--clean" -create-app -o $(BUILD_DIR)/$(APPNAME)

ifeq ($(BUILD), debug)
# create map file
LDFLAGS += -m

# create symbol files
LDFLAGS += -s
endif

### Compiler Command ###################
CC = zcc +$(TARGET) $(CFLAGS) $(SRCS) $(LDFLAGS)

### Build Target #######################
all:
	$(CC)
ifeq ($(APPTYPE), dotn)
ifeq ($(OS),Windows_NT)
	@$(RM) $(BUILD_DIR)/$(APPNAME)
	@$(MV) $(BUILD_DIR)/BINDUM $(BUILD_DIR)/$(APPNAME)
endif
endif

### Cleanup Build Files ################
clean:
	@$(RM) $(BUILD_DIR)/$(APPNAME)
	@$(RM) $(wildcard $(BUILD_DIR)/*.lis)
	@$(RM) $(wildcard $(BUILD_DIR)/*.map)
	@$(RM) $(wildcard $(BUILD_DIR)/*.sym)
	@$(RM) $(wildcard $(SRC_DIR)/*.lis)
	@$(RM) $(wildcard $(SRC_DIR)/*.sym)
	@$(RM) $(wildcard $(LIB_DIR)/libzxn/src/*.lis)
	@$(RM) $(wildcard $(LIB_DIR)/libzxn/src/*.sym) 

