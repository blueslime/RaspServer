CXX=g++
LD=g++
CFLAGS= -Wall -D_DEBUG -D__LINUX__ -DRS485  -c
LDFLAGS = -L ./Lib -lpthread -lmsr -lmad
OBJS=  main.o serial_linux.o capture.o drv-v4l.o grab-ng.o struct-v4l.o struct-dump.o command.o TCPLin.o background.o mp3.o sound.o rs422.o UDPBasic.o VideoTransit.o
all:multiflex
multiflex:$(OBJS) 
	$(LD) $(LDFLAGS) -o multiflex $(OBJS)
main.o: main.cpp type.h debuglevel.h background.h debuglevel.h
	$(CXX) $(CFLAGS) -o main.o main.cpp
background.o: background.h background.cpp debuglevel.h
	$(CXX) $(CFLAGS) -o background.o background.cpp
rs422.o: RS422/MFRS422.h RS422/MFRS422.cpp serial_linux.o debuglevel.h
	$(CXX) $(CFLAGS) -o rs422.o RS422/MFRS422.cpp
command.o: Command/command.h Command/command.cpp serial_linux.o debuglevel.h
	$(CXX) $(CFLAGS) -o command.o Command/command.cpp
serial_linux.o: SerialCom/serial_linux.cpp SerialCom/serial.h type.h debuglevel.h
	$(CXX) $(CFLAGS) -o serial_linux.o SerialCom/serial_linux.cpp
TCPLin.o:TCP/TCPLin.h TCP/TCPLin.cpp type.h debuglevel.h TCP/st_MF270.h
	$(CXX) $(CFLAGS) -o TCPLin.o TCP/TCPLin.cpp
UDPBasic.o:UDP/UDPBasic.h UDP/UDPBasic.cpp type.h debuglevel.h TCP/st_MF270.h
	$(CXX) $(CFLAGS) -o UDPBasic.o UDP/UDPBasic.cpp
capture.o: CamVision/Makefile CamVision/*.h CamVision/*.c CamVision/*.cpp debuglevel.h
	make -C CamVision/
mp3.o: Mp3_Play/*.h Mp3_Play/*.c sound.o debuglevel.h
	$(CXX) $(CFLAGS) -o mp3.o Mp3_Play/mp3_play.c
sound.o: Dev/sound.h Dev/sound.c  debuglevel.h
	$(CXX) $(CFLAGS) -o sound.o Dev/sound.c
VideoTransit.o:UDP/VideoTransit.h UDP/VideoTransit.cpp type.h debuglevel.h TCP/st_MF270.h
	$(CXX) $(CFLAGS) -o VideoTransit.o UDP/VideoTransit.cpp

install:
	cp *.o *.h Template270/.
	cp Command/*.h Template270/Command/.
	cp RS422/*.h Template270/RS422/.
	cp SerialCom/*.h Template270/SerialCom/.
	cp TCP/*.h Template270/TCP/.	
	cp UDP/*.h Template270/UDP/.
	cp CamVision/*.h Template270/CamVision/.
	cp Mp3_Play/*.h Template270/Mp3_Play/.
	cp Dev/*.h Template270/Dev/.	
	cp Lib/*.* Template270/Lib/.

clean:
	@rm -rf *.o multiflex
	@rm -rf Template270/*.* TemplateNG/multiflex
	@rm -rf Template270/Command/*.*
	@rm -rf Template270/RS422/*.*
	@rm -rf Template270/SerialCom/*.* 
	@rm -rf Template270/TCP/*.*
	@rm -rf Template270/UDP/*.*
	@rm -rf Template270/CamVision/*.*
	@rm -rf Template270/Mp3_Play/*.*
	@rm -rf Template270/Dev/*.*
