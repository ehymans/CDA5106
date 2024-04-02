CC = g++
OPT = -O3
#OPT = -g
WARN = -Wall
CFLAGS = $(OPT) $(WARN) $(INC) $(LIB)

# Since you only mentioned one .cpp file, I'll list it here. Adjust if you have more.
SIM_SRC = sim_cache.cpp

# The corresponding object file for your .cpp file
SIM_OBJ = sim_cache.o

#################################

# default rule

all: sim_cache
	@echo "finished make"

# rule for making sim_cache

sim_cache: $(SIM_OBJ)
	$(CC) -o sim_cache $(CFLAGS) $(SIM_OBJ) -lm
	@echo "-----------DONE WITH SIM_CACHE-----------"

# generic rule for converting any .cpp file to any .o file

.cpp.o:
	$(CC) $(CFLAGS) -c $*.cpp

# type "make clean" to remove all .o files plus the sim_cache binary

clean:
	rm -f *.o sim_cache

# type "make clobber" to remove all .o files (leaves sim_cache binary)

clobber:
	rm -f *.o
