#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "speechrecogn.h"

int main(int argc, char *argv[])
{
	printf("start\n");
	SpeechRecognition *sp = new SpeechRecognition;
	sp->insertItem(0, "���ʲô����", 12);
	sp->insertItem(1, "���������", 10);
	sp->insertItem(2, "��ϲ����ʲô", 12);
	sp->insertItem(3, "�ú�ѧϰ", 8);
	sp->start();
	while (getchar() != 'q')
		;
	sp->stop();
	return 0;
}
