#	Global configuration for SNAP makefiles
#

GLIB = snap/glib-core
SNAP = snap/snap-core
GLIBADV = snap/glib-adv
SNAPADV = snap/snap-adv
SNAPEXP = snap/snap-exp

CGLIB = ../$(GLIB)
CSNAP = ../$(SNAP)

EXGLIB = ../$(GLIB)
EXSNAP = ../$(SNAP)
EXGLIBADV = ../$(GLIBADV)
EXSNAPADV = ../$(SNAPADV)
EXSNAPEXP = ../$(SNAPEXP)

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
  # Linux flags
  CC = g++
  CXXFLAGS += -std=c++98 -Wall -g -fPIC
  CXXFLAGS += -O3 -DNDEBUG
  # turn on for crash debugging, get symbols with <prog> 2>&1 | c++filt
  #CXXFLAGS += -g -rdynamic
  CXXOPENMP = -fopenmp
  LDFLAGS +=
  LIBS += -lrt

else ifeq ($(UNAME), Darwin)
  # OS X flags
  CC = g++
  CXXFLAGS += -std=c++98 -Wall -g
  CXXFLAGS += -O3 -DNDEBUG
  CXXOPENMP = -fopenmp
  CLANG := $(shell g++ -v 2>&1 | grep clang | cut -d " " -f 2)
  ifeq ($(CLANG), LLVM)
    CXXOPENMP =
    CXXFLAGS += -DNOMP
  endif

  LDFLAGS +=
  LIBS +=

else ifeq ($(shell uname -o), Cygwin)
  # Cygwin flags
  CC = g++
  CXXFLAGS += -Wall -D__STDC_LIMIT_MACROS -g
  CXXFLAGS += -O3 -DNDEBUG
  CXXOPENMP = -fopenmp
  LDFLAGS +=
  LIBS +=

endif

MAIN = sosgen
DEPH = gen_logical.h gen_physical.h graph_aux.h
DEPCPP = 

OBJFILES = sosgen.o gen_logical.o gen_physical.o graph_aux.o

all: $(MAIN)

# COMPILE

sosgen: $(OBJFILES)
	$(CC) $(CXXFLAGS) -o sosgen	$(OBJFILES) $(DEPCPP) $(EXSNAP)/Snap.o -I$(EXSNAP) -I$(EXSNAPADV) -I$(EXGLIB) -I$(EXSNAPEXP) $(LDFLAGS) $(LIBS) $(CXXOPENMP)

sosgen.o:	sosgen.cpp  $(DEPH)
	$(CC) $(CXXFLAGS) -c	$(MAIN).cpp $(DEPCPP) -I$(EXSNAP) -I$(EXSNAPADV) -I$(EXGLIB) -I$(EXSNAPEXP) $(LDFLAGS) $(LIBS)

gen_logical.o:	gen_logical.cpp  $(DEPH)
	$(CC) $(CXXFLAGS) -c	gen_logical.cpp $(DEPCPP)  -I$(EXSNAP) -I$(EXSNAPADV) -I$(EXGLIB) -I$(EXSNAPEXP) $(LDFLAGS) $(LIBS)

graph_aux.o:	graph_aux.cpp  $(DEPH)
	$(CC) $(CXXFLAGS) -c	graph_aux.cpp $(DEPCPP)  -I$(EXSNAP) -I$(EXSNAPADV) -I$(EXGLIB) -I$(EXSNAPEXP) $(LDFLAGS) $(LIBS)


gen_physical.o:	gen_physical.cpp  $(DEPH)
	$(CC) $(CXXFLAGS) -c	gen_physical.cpp $(DEPCPP) -I$(EXSNAP) -I$(EXSNAPADV) -I$(EXGLIB) -I$(EXSNAPEXP) $(LDFLAGS) $(LIBS)

$(EXSNAP)/Snap.o: 
	make -C $(EXSNAP)

clean:
	rm -f *.o  $(MAIN)  $(MAIN).exe
	rm -rf Debug Release


%.pdf: %.dot
	dot -Tpdf $< -o $@    
  
run:  
	./sosgen	
	dot -Tpdf ./graphs/physical.dot -o ./graphs/physical.pdf
	dot -Tpdf ./graphs/logical0.dot -o ./graphs/logical0.pdf
	rm graphs_cplex/*.dot



