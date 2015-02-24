# Generic GNUMakefile
ifneq (,)
This makefile requires GNU Make.
endif

PROGRAM1 = roboone_hardware_server
PROGRAM2 = roboone_remaining_capacity_sync
LIB = roboone_hardware_client.a
SRC_DIR = src
OBJ_DIR = obj
API_DIR = api

SHARED_PRE = ../shared
ONEWIRE_PRE = ../OneWire
SERVER_PRE = ../MessagingServer
CLIENT_PRE = ../MessagingClient

OW_LIBS_PRE = ../OneWireLibs
OW_LIBS_FLAGS = -I $(OW_LIBS_PRE)/common -I $(OW_LIBS_PRE)/lib/userial/shared -I $(OW_LIBS_PRE)/lib/userial/link/Linux -DDEBUG
OW_LIBS_OBJ_PRE = $(OW_LIBS_PRE)/builds/uLinuxGNU/obj
OW_LIBS = $(OW_LIBS_OBJ_PRE)/owlinkobjs

C_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(C_FILES))
LIB_OBJ:= $(OBJ_DIR)/hardware_client.o $(OBJ_DIR)/hardware_msg_names.o
EXE1_OBJ:= $(OBJ_DIR)/main.o $(OBJ_DIR)/hardware_server.o $(OBJ_DIR)/orangutan.o $(OBJ_DIR)/ow_bus.o $(OBJ_DIR)/hardware_msg_names.o
EXE2_OBJ:= $(OBJ_DIR)/remaining_capacity_sync.o
CC = $(GCC_PREFIX)gcc.exe
AR =  $(GCC_PREFIX)ar.exe
DFLAGS = -O0 -fbuiltin -g
CFLAGS = -O2 -Wall -pedantic -pedantic-errors -I. -I$(SRC_DIR) -I$(API_DIR) -I$(SHARED_PRE)/$(API_DIR) -I$(ONEWIRE_PRE)/$(API_DIR) -I$(SERVER_PRE)/$(API_DIR) -I$(CLIENT_PRE)/$(API_DIR) $(OW_LIBS_FLAGS)
LDFLAGS = $(SHARED_PRE)/$(OBJ_DIR)/shared.a  $(ONEWIRE_PRE)/$(OBJ_DIR)/one_wire.a  $(OW_LIBS) $(SERVER_PRE)/$(OBJ_DIR)/messaging_server.a

all: $(PROGRAM1) $(PROGRAM2)

$(PROGRAM1): $(LIB)
	$(CC) $(EXE1_OBJ) $(LDFLAGS) -o $(OBJ_DIR)/$(PROGRAM1)

$(PROGRAM2): $(LIB)
	$(CC) $(EXE2_OBJ) $(LDFLAGS) $(OBJ_DIR)/$(LIB) $(CLIENT_PRE)/$(OBJ_DIR)/messaging_client.a -o $(OBJ_DIR)/$(PROGRAM2)

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
	rm -f depend $(OBJ_DIR)/.depend $(OBJ_DIR)/*.o $(OBJ_DIR)/$(LIB) $(OBJ_DIR)/$(PROGRAM1) $(OBJ_DIR)/$(PROGRAM2)

.PHONY: clean depend