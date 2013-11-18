CPP=g++
CCFLAGS=-pipe -Wall -Wno-deprecated $(DEFS) -g -O3

.SUFFIXES: .o .C .d
.C.o:
	$(CPP) -c $(CCFLAGS) $<
.C.d:
	$(CPP) -MM $(CCFLAGS) -MF $@ $<

EXE=tsp_lns

HEADERS= lns_space.h lns.h meta_lns.h

SRCS = tsp_lns.C lns.C meta_lns.C

OBJS=$(SRCS:.C=.o)

DEPS=$(SRCS:.C=.d)

%.o: %.C $(DEPS)
	$(CXX) -c -o $@ $< $(CCFLAGS)

$(EXE): $(OBJS) 
	$(CPP) -framework Gecode $(CCFLAGS) -o $(EXE) $(OBJS) $(LDEFS)

all: $(EXE)

	
clean:
	rm -f $(OBJS) $(DEPS) $(EXE) *.o

sinclude $(DEPS)
