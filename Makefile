CC = g++
CXX = g++

OBJS = logger.o testset.o launcher.o proc.o cpu_usage.o meminfo.o main.o
TARGET = run_test

CXXFLAGS = -std=c++11 -O2 -g `pkg-config --cflags json-glib-1.0`
LDFLAGS = -lstdc++ `pkg-config --libs json-glib-1.0`

.SUFFIXES : .c .o
 
all : $(TARGET)

$(TARGET): $(OBJS)
	   $(CC) -o $@ $(OBJS) $(LDFLAGS)
 
clean :
	rm -f $(OBJS) $(TARGET)
