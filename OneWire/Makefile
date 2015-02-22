# Generic GNUMakefile
ifneq (,)
This makefile requires GNU Make.
endif

PROGRAM = one_wire
LIB = one_wire.a
SRC_DIR = src
OBJ_DIR = obj
API_DIR = api

SHARED_PRE = ../shared

OW_LIBS_PRE = ../OneWireLibs
OW_LIBS_FLAGS = -I$(OW_LIBS_PRE)/common -I$(OW_LIBS_PRE)/lib/userial/shared -I$(OW_LIBS_PRE)/lib/userial/link/Linux -DDEBUG
OW_LIBS_OBJ_PRE = $(OW_LIBS_PRE)/builds/uLinuxGNU/obj
OW_LIBS = $(OW_LIBS_OBJ_PRE)/owlinkobjs

C_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(C_FILES))
LIB_OBJ:= $(OBJ_DIR)/ds2438.o $(OBJ_DIR)/ds2408.o $(OBJ_DIR)/utils.o
TST_OBJ:= $(OBJ_DIR)/test.o
CC = $(GCC_PREFIX)gcc.exe
AR =  $(GCC_PREFIX)ar.exe
DFLAGS = -O0 -fbuiltin -g
CFLAGS = -O2 -Wall -pedantic -pedantic-errors -I. -I$(SRC_DIR) -I$(API_DIR) -I$(SHARED_PRE)/$(API_DIR) $(OW_LIBS_FLAGS)
LDFLAGS = $(OBJ_DIR)/$(LIB) $(OW_LIBS) $(OW_LIBS_OBJ_PRE)/findtype.o $(SHARED_PRE)/$(OBJ_DIR)/shared.a

all: $(PROGRAM)

$(PROGRAM): $(LIB)
	$(CC) $(TST_OBJ) $(LDFLAGS) -o $(OBJ_DIR)/$(PROGRAM)

$(LIB): .depend $(OBJS)
	$(AR) r $(OBJ_DIR)/$(LIB) $(LIB_OBJ)

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
	rm -f depend $(OBJ_DIR)/.depend $(OBJ_DIR)/*.o $(OBJ_DIR)/*.a $(OBJ_DIR)/$(PROGRAM) $(OBJ_DIR)/$(LIB)

.PHONY: clean depend