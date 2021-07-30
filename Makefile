CPPFLAGS=-g -O3 -Werror -Wall -Wno-psabi -mtune=native -march=native -std=c++11
ifndef OPT
CPPFLAGS += -DLOGGING -DCOUNTERS -DTIMERS
endif
LDFLAGS=-g
LDLIBS=
RM=rm -f

all: bin/xc bin/xcc

bin/xc: src/xc.cc src/logging.h src/counters.h src/flags.h
	g++ $(CPPFLAGS) -o bin/xc src/xc.cc $(LDLIBS)

bin/xcc: src/xcc.cc src/logging.h src/counters.h src/flags.h
	g++ $(CPPFLAGS) -o bin/xcc src/xcc.cc $(LDLIBS)

clean:
	$(RM) bin/xc
	$(RM) bin/xcc
	$(RM) *~
	$(RM) */*~
