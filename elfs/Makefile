# comments begin with pound sign.
CXX = g++ # an alias. You can specify any number of aliases
CF = -Wall -mno-red-zone -nostdlib -static -O0
 
all: ret3 square fibb # This is first target. It is used when no params passed to “make”

ret3: ret3.o 
	$(CXX) $(CF) ret3.o -o ret3 # command builds ‘ret3’

ret3.o : ret3.cpp
	$(CXX) $(CF) -c ret3.cpp


square: square.o 
	$(CXX) $(CF) square.o -o square # command builds ‘square’

square.o : square.cpp
	$(CXX) $(CF) -c square.cpp
	
	
fibb: fibb.o 
	$(CXX) $(CF) fibb.o -o fibb # command builds ‘fibb’

fibb.o : fibb.cpp
	$(CXX) $(CF) -c fibb.cpp
	
# indicate that following target doesnt update any file called "clean"
.PHONY: clean

clean:
	rm -rf *o ret3 square fibb	# remove all build produsts
	@echo Removed all build results # @echo prints out text