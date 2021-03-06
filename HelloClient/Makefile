# Generic GNUMakefile
ifneq (,)
This makefile requires GNU Make.
endif

PROGRAM = client.out
SRC_DIR = src
OBJ_DIR = obj
API_DIR = api

SHARED_PRE = ../shared

C_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(C_FILES))
CC =  $(GCC_PREFIX)gcc.exe
CFLAGS = -Wall -pedantic -pedantic-errors -I. -I$(SRC_DIR) -I$(API_DIR) -I$(SHARED_PRE)/$(API_DIR)
LDFLAGS = 

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
	$(CC) $(CFLAGS) -c $< -o $@

%:$(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f depend $(OBJ_DIR)/.depend $(OBJ_DIR)/*.o $(OBJ_DIR)/$(PROGRAM)

.PHONY: clean depend