# comments begin with pound sign.
CXX = g++ # an alias. You can specify any number of aliases
CF = -std=c++11 -Wall -mno-red-zone -nostartfiles -static -O0
SOURCES = ret3.cpp square.cpp fibb.cpp sum.cpp msg.cpp
TARGETS = $(SOURCES:.cpp=)

all: $(TARGETS) # This is first target. It is used when no params passed to “make”

%: %.cpp
	$(CXX) $(CF) $< -o $@
	

install: $(TARGETS)
	@cp $^ ../p2
	@sync
	 
# indicate that following target doesnt update any file called "clean"
.PHONY: clean

clean:
	rm -rf $(OBJECTS) $(TARGETS)	# remove all build produsts
	@echo Removed all build results # @echo prints out text