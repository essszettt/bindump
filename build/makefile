### Target Platform ####################
TARGET = zxn

### Project Name #######################
APPNAME = bindump

### Source Directories #################
LIB_DIR = ../lib
SRC_DIR = ../src
INC_DIR = ../inc
BUILD_DIR = .

### Source Files #######################
SRCS  = $(SRC_DIR)/main.c
SRCS += $(LIB_DIR)/libzxn/src/libzxn.c
SRCS += $(LIB_DIR)/libzxn/src/libzxn.asm

### Compiler Flags #####################
CFLAGS = -compiler=sdcc --vc -clib=sdcc_iy -SO3 --opt-code-size -I$(INC_DIR) -I$(LIB_DIR)/libzxn/inc -pragma-include:$(INC_DIR)/zpragma.inc

# Select CRT
CFLAGS += -startup=30

# tradeoff speed vs. code quality
CFLAGS += --max-allocs-per-node200000

# verbose output
# CFLAGS += -v

# create list files
# CFLAGS += --list

# create debug code
# CFLAGS += -D__DEBUG__

### Linker Flags #######################
LDFLAGS = -subtype=dotn -Cz"--clean" -create-app -o $(BUILD_DIR)/$(APPNAME) 

# create map file
# LDFLAGS += -m

# create symbol files
# LDFLAGS += -s

### Compiler Command ###################
CC = zcc +$(TARGET) $(CFLAGS) $(SRCS) $(LDFLAGS)

### Build Target #######################
all:
	$(CC)
	@rm -f $(BUILD_DIR)/$(APPNAME)
	@mv -f $(BUILD_DIR)/BINDUM $(BUILD_DIR)/$(APPNAME)

### Cleanup Build Files ################
clean:
	@rm -f $(BUILD_DIR)/$(APPNAME)
	@rm -f $(BUILD_DIR)/$(APPNAME).lis
	@rm -f $(BUILD_DIR)/$(APPNAME).map
	@rm -f $(BUILD_DIR)/$(APPNAME).sym
	@rm -f $(SRC_DIR)/*.lis
	@rm -f $(SRC_DIR)/*.sym
