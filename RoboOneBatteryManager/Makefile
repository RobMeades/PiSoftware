# Generic GNUMakefile
ifneq (,)
This makefile requires GNU Make.
endif

PROGRAM = roboone_battery_manager_server
LIB = roboone_battery_manager_client.a
SRC_DIR = src
OBJ_DIR = obj
API_DIR = api

SHARED_PRE = ../shared
ROBOONEHARDWARE_PRE = ../RoboOneHardware
ROBOONESTATEMACHINE_PRE = ../RoboOneStateMachine
ROBOONETASKHANDLER_PRE = ../RoboOneTaskHandler
SERVER_PRE = ../MessagingServer
CLIENT_PRE = ../MessagingClient

C_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(C_FILES))
LIB_OBJ:= $(OBJ_DIR)/battery_manager_client.o $(OBJ_DIR)/battery_manager_msg_names.o
EXE_OBJ:= $(OBJ_DIR)/main.o $(OBJ_DIR)/battery_manager_server.o $(OBJ_DIR)/battery_manager_msg_names.o
CC = $(GCC_PREFIX)gcc.exe
AR =  $(GCC_PREFIX)ar.exe
CFLAGS = -Wall -pedantic -pedantic-errors -I. -I$(SRC_DIR) -I$(API_DIR) -I$(SHARED_PRE)/$(API_DIR) -I$(ROBOONEHARDWARE_PRE)/$(API_DIR) -I$(ROBOONESTATEMACHINE_PRE)/$(API_DIR) -I$(ROBOONETASKHANDLER_PRE)/$(API_DIR) -I$(SERVER_PRE)/$(API_DIR) -I$(CLIENT_PRE)/$(API_DIR)
LDFLAGS = $(OBJ_DIR)/$(LIB) $(SHARED_PRE)/$(OBJ_DIR)/shared.a $(ROBOONEHARDWARE_PRE)/$(OBJ_DIR)/roboone_hardware_client.a $(ROBOONESTATEMACHINE_PRE)/$(OBJ_DIR)/roboone_state_machine_client.a $(ROBOONETASKHANDLER_PRE)/$(OBJ_DIR)/roboone_task_handler_client.a $(SERVER_PRE)/$(OBJ_DIR)/messaging_server.a $(CLIENT_PRE)/$(OBJ_DIR)/messaging_client.a

all: $(PROGRAM)

$(PROGRAM): $(LIB)
	$(CC) $(EXE_OBJ) $(LDFLAGS) -o $(OBJ_DIR)/$(PROGRAM)

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
	rm -f depend $(OBJ_DIR)/.depend $(OBJ_DIR)/*.o $(OBJ_DIR)/$(LIB) $(OBJ_DIR)/$(PROGRAM)

.PHONY: clean depend