CC = g++
OPT = -O3
#OPT = -g
WARN = -Wall
CFLAGS = $(OPT) $(WARN) $(INC) $(LIB)

SIM_SRC = sim_cache.cpp

# Output the object file 
SIM_OBJ = sim_cache.o

#################################

# default rule

all: sim_cache
	@echo "finished make"

# rule for making sim_cache

sim_cache: $(SIM_OBJ)
	$(CC) -o sim_cache $(CFLAGS) $(SIM_OBJ) -lm
	@echo "-----------DONE WITH SIM_CACHE-----------"

# rule to convert  cpp to .o

.cpp.o:
	$(CC) $(CFLAGS) -c $*.cpp

# "make clean" removes all .o files plus the sim_cache binary

clean:
	rm -f *.o sim_cache

# "make clobber" removes all .o files (leaves sim_cache binary)

clobber:
	rm -f *.o
