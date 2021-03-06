# Generic GNUMakefile
ifneq (,)
This makefile requires GNU Make.
endif

PROGRAM = charger
SRC_DIR = src
OBJ_DIR = obj
API_DIR = api
FLAGS = 

SHARED_PRE = ../shared
PI_IO_PRE =  ../PiIo
ONEWIRE_PRE = ../OneWire

OW_LIBS_PRE = ../OneWireLibs
OW_LIBS_FLAGS = -I $(OW_LIBS_PRE)/common -I $(OW_LIBS_PRE)/lib/userial/shared -I $(OW_LIBS_PRE)/lib/userial/link/Linux -DDEBUG
OW_LIBS_OBJ_PRE = $(OW_LIBS_PRE)/builds/uLinuxGNU/obj
OW_LIBS = $(OW_LIBS_OBJ_PRE)/owlinkobjs

C_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(C_FILES))
CC = $(GCC_PREFIX)gcc.exe
DFLAGS = -O0 -fbuiltin -g
CFLAGS = -O2 -Wall -pedantic -pedantic-errors -I. -I$(SRC_DIR) -I$(API_DIR) -I$(SHARED_PRE)/$(API_DIR) -I$(PI_IO_PRE)/$(API_DIR) $(FLAGS) -I$(ONEWIRE_PRE)/$(API_DIR) $(OW_LIBS_FLAGS)
LDFLAGS = -lncurses -pthread $(SHARED_PRE)/$(OBJ_DIR)/shared.a $(PI_IO_PRE)/$(OBJ_DIR)/pi_io.a  $(ONEWIRE_PRE)/$(OBJ_DIR)/one_wire.a  $(OW_LIBS_OBJ_PRE)/findtype.o $(OW_LIBS) 

all: $(PROGRAM)

$(PROGRAM): .depend $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(OBJ_DIR)/$(PROGRAM)

depend: .depend

.depend: cmd = $(CC) -MM -MF depend $(var); cat depend >> $(OBJ_DIR)/.depend;
.depend:
	@echo "Generating dependencies..."
	@mkdir -p $(OBJ_DIR)
	@$(foreach var, $(C_FILES), $(cmd))
	@rm -f depend

-include .depend

# These are the pattern matching rules. In addition to the automatic
# variables used here, the variable $* that matches whatever % stands for
# can be useful in special cases.
$(OBJ_DIR)/%.o:$(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

%:$(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f depend $(OBJ_DIR)/.depend $(OBJ_DIR)/*.o $(OBJ_DIR)/$(PROGRAM)

.PHONY: clean depend