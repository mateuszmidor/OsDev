# comments begin with pound sign.
CXX = g++ # an alias. You can specify any number of aliases
CF = -Wall -mno-red-zone -nostdlib -static -O0
SOURCES = ret3.cpp square.cpp fibb.cpp
TARGETS = $(SOURCES:.cpp=)

all: $(TARGETS) # This is first target. It is used when no params passed to “make”

%: %.cpp
	$(CXX) $(CF) $< -o $@
	

install: $(TARGETS)
	@cp $^ ../p2
	 
# indicate that following target doesnt update any file called "clean"
.PHONY: clean

clean:
	rm -rf $(OBJECTS) $(TARGETS)	# remove all build produsts
	@echo Removed all build results # @echo prints out text