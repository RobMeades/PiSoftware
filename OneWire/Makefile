# Generic GNUMakefile
ifneq (,)
This makefile requires GNU Make.
endif

PROGRAM = a.out
SRC_DIR = src
OBJ_DIR = obj
API_DIR = api
C_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(C_FILES))
CC = gcc.exe
CFLAGS = -Wall -pedantic -std=c99 -I. -I$(SRC_DIR) -I$(API_DIR)
LDFLAGS =

all: $(PROGRAM)

$(PROGRAM): .depend $(OBJS)
	$(GCC_PREFIX)$(CC) $(LDFLAGS) $(OBJS) -o $(OBJ_DIR)/$(PROGRAM)

depend: .depend

.depend: cmd = $(GCC_PREFIX)gcc.exe -MM -MF depend $(var); cat depend >> $(OBJ_DIR)/.depend;
.depend:
	@echo "Generating dependencies..."
	@$(foreach var, $(C_FILES), $(cmd))
	@rm -f depend

-include .depend

# These are the pattern matching rules. In addition to the automatic
# variables used here, the variable $* that matches whatever % stands for
# can be useful in special cases.
$(OBJ_DIR)/%.o:$(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(GCC_PREFIX)$(CC) $(CFLAGS) -c $< -o $@

%:$(SRC_DIR)/%.c
	$(GCC_PREFIX)$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(OBJ_DIR)/.depend $(OBJ_DIR)/*.o $(OBJ_DIR)/$(PROGRAM)

.PHONY: clean depend