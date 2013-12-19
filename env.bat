@echo off
set CYGWINDIR=D:\temp\MFCodeEditor\CygWin\bin
set CROSSCPDIR=D:\temp\MFCodeEditor\CygWin\opt\gcc-3.4.6-glibc-2.3.6\arm-linux\bin

set PATH=%PATH%;D:\temp\MFCodeEditor\MinGW\bin
set PATH=%PATH%;%SystemRoot%\System32
set PATH=%PATH%;%CYGWINDIR%;%CROSSCPDIR%

echo Setting up a Qt ARM only environment...
echo -- PATH set to %PATH%
