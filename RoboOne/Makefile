# Generic GNUMakefile
ifneq (,)
This makefile requires GNU Make.
endif

PROGRAM = roboone
SRC_DIR = src
OBJ_DIR = obj
API_DIR = api
FLAGS = 

SHARED_PRE = ../shared
TIMER_PRE = ../timer
SERVER_PRE = ../MessagingServer
CLIENT_PRE = ../MessagingClient
ROBOONEHARDWARE_PRE = ../RoboOneHardware
ROBOONEBATTERYMANAGER_PRE = ../RoboOneBatteryManager
ROBOONESTATEMACHINE_PRE = ../RoboOneStateMachine
ROBOONETASKHANDLER_PRE = ../RoboOneTaskHandler

C_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(C_FILES))
CC = $(GCC_PREFIX)gcc.exe
DFLAGS = -O0 -fbuiltin -g
CFLAGS = -O2 -Wall -pedantic -pedantic-errors -I. -I$(SRC_DIR) -I$(API_DIR) -I$(SHARED_PRE)/$(API_DIR) -I$(TIMER_PRE)/$(API_DIR) -I$(SERVER_PRE)/$(API_DIR) -I$(CLIENT_PRE)/$(API_DIR) -I$(ROBOONEHARDWARE_PRE)/$(API_DIR) -I$(ROBOONEBATTERYMANAGER_PRE)/$(API_DIR) -I$(ROBOONESTATEMACHINE_PRE)/$(API_DIR) -I$(ROBOONETASKHANDLER_PRE)/$(API_DIR) $(FLAGS)
LDFLAGS = -lncurses -pthread $(SHARED_PRE)/$(OBJ_DIR)/shared.a $(TIMER_PRE)/$(OBJ_DIR)/timer_client.a $(OW_LIBS) $(ROBOONEHARDWARE_PRE)/$(OBJ_DIR)/roboone_hardware_client.a $(ROBOONEBATTERYMANAGER_PRE)/$(OBJ_DIR)/roboone_battery_manager_client.a $(CLIENT_PRE)/$(OBJ_DIR)/messaging_client.a  $(SERVER_PRE)/$(OBJ_DIR)/messaging_server.a $(ROBOONESTATEMACHINE_PRE)/$(OBJ_DIR)/roboone_state_machine_client.a $(ROBOONETASKHANDLER_PRE)/$(OBJ_DIR)/roboone_task_handler_client.a

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