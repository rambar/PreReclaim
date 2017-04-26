CC = g++
CXX = g++

OBJS = testset.o launcher.o proc.o cpu_usage.o meminfo.o main.o
TARGET = pre-reclaim
 
CXXFLAGS = -std=c++11 -O2 -g
LDFLAGS = -lstdc++

.SUFFIXES : .c .o
 
all : $(TARGET)

$(TARGET): $(OBJS)
	   $(CC) -o $@ $(OBJS)
 
clean :
	rm -f $(OBJS) $(TARGET)
