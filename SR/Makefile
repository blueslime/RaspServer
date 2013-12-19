CC=arm-linux-gcc
CXX=arm-linux-g++
CFLAGS = -c -g -D_DEBUG -D__LINUX__ -I ../
LDFLAGS = -lmsr -lpthread #-lxml2 -lmad -lpthread -lmsr

all:
	$(CXX) $(CFLAGS) voice.c -o ../voice.o
	$(CXX) $(CFLAGS) speechrecogn.cpp -o ../speechrecogn.o

clean:
	rm -f speechrecogn *.o *~ *.bak
