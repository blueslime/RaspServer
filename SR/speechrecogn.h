//////////////////////////////////////////////////////////////////////////
// for MultiflexNG
// http://robot.up-tech.com 
// Designed by lsy
// Modified by zwj @ 20090317
// zwj_uptech@126.com
//////////////////////////////////////////////////////////////////////////
#ifndef SPEECH_RECOGN_H
#define SPEECH_RECOGN_H

class SpeechRecognition
{
public:
	bool Pause();
	bool Continue();
	bool isStarted();
	void ResetResIndex();
	int GetResultIndex();
	enum {MAX_INDEX=20, MAX_ITEM_SIZE=32};
	SpeechRecognition();
	virtual ~SpeechRecognition();
	void start();
	void stop();
	void insertItem(int index, const char *item, int size);		//插入词条

	void replaceItem(int index, const char *item, int size);	//更换词条
	void deleteItem(int index);
	void deleteItem(int first, int last);
	void clear();												//清空词条
	void textReturned(const char *text);
protected:
	void sendResult(int index);
	void addVocabularyToEngine();
private:
	char m_szVoc[MAX_INDEX][MAX_ITEM_SIZE];
	int m_nItem;
	char m_szDsp[32];
	bool m_bIsStarted;
	int m_channel;
	int m_nResIndex;
//static void *voiceThread(void *arg);
//static void *parseThread(void *arg);
};

#endif //SPEECH_RECOGN_H
