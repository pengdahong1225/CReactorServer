CC  = gcc
CXX = g++
GCCVER := $(shell $(CC) -dumpversion | awk -F. '{ print $$1"."$$2}' )

OPT = -pipe -fno-ident -fPIC -shared -z defs
CFLAGS += $(OPT) -g -pg -Wall -D_GNU_SOURCE -funroll-loops -MMD -D_REENTRANT -Wno-invalid-offsetof
ifeq ($(MEMCHECK),1)
CFLAGS += -DMEMCHECK
endif

CXXFLAGS = -std=c++2a -Wall -g -pipe -rdynamic -fno-strict-aliasing -Wno-unused-function -Wno-sign-compare -fpermissive -Wno-invalid-offsetof
CXXFLAGS += $(CFLAGS)

LINK = -lpthread

INC	= -I. -I./Base -I./Common  -I./Net -I./Net/Poller
SRCS = $(wildcard ./*.cpp) $(wildcard Base/*.cpp) $(wildcard Common/*.cpp) $(wildcard Net/*.cpp) $(wildcard Net/Poller/*.cpp)

COMPILE_LIB_HOME = ./
DYNAMIC_NAME = libreactor.so
STATIC_NAME = libreactor.a
DYNAMIC_LIB	= $(COMPILE_LIB_HOME)/$(DYNAMIC_NAME)
STATIC_LIB = $(COMPILE_LIB_HOME)/$(STATIC_NAME)

all: $(DYNAMIC_LIB) $(STATIC_LIB)

$(DYNAMIC_LIB): $(SRCS:.cpp=.o) 
	$(CXX) -pg -o $@ $^ $(CXXFLAGS) $(LINK)

$(STATIC_LIB): $(SRCS:.cpp=.o)
	@ar cr $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c -pg -o $@ $<
%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INC) -c -pg -o $@ $<
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INC) -c -pg -o $@ $<

clean:
	rm -f *.o .po *.so *.d .dep.*  $(SRCS:.cpp=.o) $(SRCS:.cpp=.d) $(DYNAMIC_LIB) $(STATIC_LIB) $(DYNAMIC_NAME) $(STATIC_NAME)
