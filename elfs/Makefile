# comments begin with pound sign.
CXX = g++ # an alias. You can specify any number of aliases
CF = -std=c++11 -Wall -mno-red-zone -nostartfiles -static -O0 -fdata-sections -ffunction-sections -Wl,--gc-sections
SOURCES = ret3.cpp square.cpp fibb.cpp sum.cpp msg.cpp smem.cpp dmem.cpp cat.cpp cp.cpp dmesg.cpp
GCCINCLUDES = -I../src/middlespace

TARGETS = $(SOURCES:.cpp=)

all: $(TARGETS) # This is first target. It is used when no params passed to “make”
	$(MAKE) -C terminal

%: %.cpp
	$(CXX) $(CF) $(GCCINCLUDES) $< -o $@
	

install: $(TARGETS)
	@cp $^ ../p1
	$(MAKE) install -C terminal
	@sync
	 
# indicate that following target doesnt update any file called "clean"
.PHONY: clean

clean:
	rm -rf $(OBJECTS) $(TARGETS)	# remove all build produsts
	$(MAKE) clean -C terminal
	@echo Removed all build results # @echo prints out text