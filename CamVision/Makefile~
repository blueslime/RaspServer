CC=gcc
CXX=g++
CFLAGS = -c -g -D_DEBUG -D__LINUX__ -I ../
LDFLAGS = -lmsr -lpthread 
#-lxml2 -lmad -lpthread -lmsr
#EXTRA_LIBS += -lpthread

all:
	$(CC) $(CFLAGS) drv-v4l.c -o ../drv-v4l.o
	$(CC) $(CFLAGS) grab-ng.c -o ../grab-ng.o
	$(CC) $(CFLAGS) struct-v4l.c -o ../struct-v4l.o
	$(CC) $(CFLAGS) struct-dump.c  -o ../struct-dump.o
	#$(CC) $(CFLAGS) struct-dump.c drv-v4l.c -o ../v4lcap.o
	$(CXX) $(CFLAGS) -o ../capture.o capture.cpp 


clean:
	#rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions $(TARGET)
	rm -rf *.o *.gdb .tmp_versions $(TARGET)
