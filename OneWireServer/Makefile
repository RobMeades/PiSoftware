# Generic GNUMakefile
ifneq (,)
This makefile requires GNU Make.
endif

PROGRAM = one_wire_server
LIB = one_wire_server.a
SRC_DIR = src
OBJ_DIR = obj
API_DIR = api
FLAGS =

SHARED_PRE = ../shared
ONEWIRE_PRE = ../OneWire
SERVER_PRE = ../MessagingServer

OW_LIBS_PRE = ../OneWireLibs
OW_LIBS_FLAGS = -I $(OW_LIBS_PRE)/common -I $(OW_LIBS_PRE)/lib/userial/shared -I $(OW_LIBS_PRE)/lib/userial/link/Linux -DDEBUG
OW_LIBS_OBJ_PRE = $(OW_LIBS_PRE)/builds/uLinuxGNU/obj
OW_LIBS = $(OW_LIBS_OBJ_PRE)/owlinkobjs

C_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(C_FILES))
LIB_OBJ:= $(OBJ_DIR)/one_wire_server.o
TST_OBJ:= $(OBJ_DIR)/main.o
CC =  $(GCC_PREFIX)gcc.exe
AR =  $(GCC_PREFIX)ar.exe
CFLAGS = -Wall -pedantic -pedantic-errors -I. -I$(SRC_DIR) -I$(API_DIR) -I$(SHARED_PRE)/$(API_DIR) -I$(ONEWIRE_PRE)/$(API_DIR) -I$(SERVER_PRE)/$(API_DIR) $(OW_LIBS_FLAGS) $(FLAGS)
# In the line below $(LIB_OBJ) should be $(OBJ_DIR)/$(LIB) (to use the library) but for some reason this doesn't work (even though the library does work elsewher)
LDFLAGS = $(LIB_OBJ) $(SHARED_PRE)/$(OBJ_DIR)/shared.a  $(ONEWIRE_PRE)/$(OBJ_DIR)/one_wire.a  $(OW_LIBS) $(SERVER_PRE)/$(OBJ_DIR)/messaging_server.a

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
	$(CC) $(CFLAGS) -c $< -o $@

%:$(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f depend $(OBJ_DIR)/.depend $(OBJ_DIR)/*.o $(OBJ_DIR)/$(PROGRAM)

.PHONY: clean depend