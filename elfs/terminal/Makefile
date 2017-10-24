# comments begin with pound sign.
CXX = g++ # an alias. You can specify any number of aliases
CFLAGS = -std=c++11 -Wall -mno-red-zone -nostartfiles -static -O0
SOURCES = TerminalEnv.cpp main.cpp
INCLUDES = -I. -Icmds -I.. -I../../src/middlespace

COMMANDS := cmds/CmdBase.cpp cmds/pwd.cpp cmds/ls.cpp cmds/elfrun.cpp cmds/cd.cpp #$(wildcard cmds/*.cpp) 
SOURCES = main.cpp TerminalEnv.cpp Cursor.cpp CommandHistory.cpp CommandCollection.cpp CommandLineInput.cpp \
			LineBuffer.cpp ScrollableScreenPrinter.cpp \
			$(COMMANDS) Terminal.cpp

		
OBJECTS=$(SOURCES:.cpp=.o) # simply change extensions from .cpp to .o
MAIN = terminal # output executable name

all: $(MAIN) # default target is MAIN

# how to build MAIN
# $@ holds $(MAIN), $^ holds $(OBJECTS) ie. two sides of the colon “:”
$(MAIN): $(OBJECTS)
	$(CXX) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBPATHS) $(LIBS)

# how to turn any .cpp file into .o file; $< is input .cpp, $@ is output .o
.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@
        
        
install: $(MAIN)
	@cp $^ ../../p1
	@sync
	 
# indicate that following target doesnt update any file called "clean"
.PHONY: clean

clean:
	rm -rf $(OBJECTS) $(TARGETS) $(MAIN)	# remove all build produsts
	@echo Removed all build results # @echo prints out text