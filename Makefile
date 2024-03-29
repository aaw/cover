CPPFLAGS=-g -O3 -Werror -Wall -Wno-psabi -mtune=native -march=native -std=c++20
ifndef OPT
CPPFLAGS += -DLOGGING -DCOUNTERS -DTIMERS
endif
LDFLAGS=-g
LDLIBS=
RM=rm -f

all: bin/xc bin/xcc bin/mcc bin/dc

bin/xc: src/xc.cc src/logging.h src/counters.h src/flags.h src/params.h
	g++ $(CPPFLAGS) -o bin/xc src/xc.cc $(LDLIBS)

bin/xcc: src/xcc.cc src/logging.h src/counters.h src/flags.h src/params.h
	g++ $(CPPFLAGS) -o bin/xcc src/xcc.cc $(LDLIBS)

bin/mcc: src/mcc.cc src/logging.h src/counters.h src/flags.h src/params.h
	g++ $(CPPFLAGS) -o bin/mcc src/mcc.cc $(LDLIBS)

bin/dc: src/dc.cc src/logging.h src/counters.h src/flags.h src/params.h
	g++ $(CPPFLAGS) -o bin/dc src/dc.cc $(LDLIBS)

clean:
	$(RM) bin/xc
	$(RM) bin/xcc
	$(RM) bin/mcc
	$(RM) bin/dc
	$(RM) *~
	$(RM) */*~
