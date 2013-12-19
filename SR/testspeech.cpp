#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "speechrecogn.h"

int main(int argc, char *argv[])
{
	printf("start\n");
	SpeechRecognition *sp = new SpeechRecognition;
	sp->insertItem(0, "你叫什么名字", 12);
	sp->insertItem(1, "你从哪里来", 10);
	sp->insertItem(2, "你喜欢做什么", 12);
	sp->insertItem(3, "好好学习", 8);
	sp->start();
	while (getchar() != 'q')
		;
	sp->stop();
	return 0;
}
