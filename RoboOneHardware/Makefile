# Generic GNUMakefile
ifneq (,)
This makefile requires GNU Make.
endif

LIB = roboone_hardware.a
SRC_DIR = src
OBJ_DIR = obj
API_DIR = api

SHARED_PRE = ../shared
ONEWIRE_PRE = ../OneWire
SERVER_PRE = ../MessagingServer
CLIENT_PRE = ../MessagingClient
ONEWIRESERVER_PRE = ../OneWireServer

C_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(C_FILES))
CC = $(GCC_PREFIX)gcc.exe
AR =  $(GCC_PREFIX)ar.exe
CFLAGS = -Wall -pedantic -pedantic-errors -I. -I$(SRC_DIR) -I$(API_DIR) -I$(SHARED_PRE)/$(API_DIR) -I$(ONEWIRE_PRE)/$(API_DIR) -I$(SERVER_PRE)/$(API_DIR) -I$(CLIENT_PRE)/$(API_DIR) -I$(ONEWIRESERVER_PRE)/$(API_DIR)

all: $(LIB)

$(LIB): .depend $(OBJS)
	$(AR) r $(OBJ_DIR)/$(LIB) $(OBJS)

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
	rm -f depend $(OBJ_DIR)/.depend $(OBJ_DIR)/*.o $(OBJ_DIR)/$(LIB)

.PHONY: clean depend