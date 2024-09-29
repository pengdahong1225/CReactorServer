CXX = g++

TARGET = Server
CXXFLAGS = -std=c++2a -Wall -g -pipe -rdynamic -fno-strict-aliasing -Wno-unused-function -Wno-sign-compare -fpermissive -Wno-invalid-offsetof

LINK = -L. -l:libreactor.a

INC	= -I. -I../ -I../Common  -I../Net -I../Net/Poller 
SRCS = $(wildcard ./*.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))

all: $(TARGET)

$(TARGET) : $(OBJS)
	 $(CXX) $(CXXFLAGS) -o $@ $^ $(LINK) -lpthread
%.o:%.cpp
	$(CXX) $(CXXFLAGS) $(INC) -c $^ -o $@

.PHONY:clean
clean:
	rm $(OBJS) $(TARGET)