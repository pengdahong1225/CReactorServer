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

PROJECT_DIR := ..
LIB_DIR	:= $(PROJECT_DIR)/libs
DYNAMIC_NAME = libcore.so
STATIC_NAME = libcore.a
DYNAMIC_LIB	= $(LIB_DIR)/$(DYNAMIC_NAME)
STATIC_LIB = $(LIB_DIR)/$(STATIC_NAME)

INC	= -I./include -I../3rdparty
SRCS = $(wildcard ./src/*.cpp)
OBJS = $(patsubst %.cpp, %.o,  $(SRCS))
LINK = -lpthread
LINK += -L$(LIB_DIR) -lglog

# objdep
DEP_DIR := ./objdep
SUBDIRS=$(shell ls -l | grep ^d | awk '{print $$9}')
SUBDIRS+=$(shell ls -l | grep ^l | awk '{print $$9}')
$(shell if [ ! -d $(DEP_DIR) ]; then for dir in $(SUBDIRS); do mkdir -p $(DEP_DIR)/$$dir; done fi)

OBJS = $(addprefix $(DEP_DIR)/, $(patsubst %.cpp, %.o, $(SRCS)))

all: $(DYNAMIC_LIB) $(STATIC_LIB)

$(DYNAMIC_LIB): $(OBJS) 
	$(CXX) -pg -o $@ $^ $(CXXFLAGS) $(LINK)

$(STATIC_LIB): $(OBJS)
	@ar cr $@ $^

$(OBJS) :$(DEP_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INC) -c -pg -o $@ $<

clean:
	rm -rf $(DEP_DIR) $(DYNAMIC_LIB) $(STATIC_LIB)
