#ifndef           __VOICE
#define           __VOICE
int msr_init();
int voice_init();
void voice_exit();
int voice_stop(int wait);
int voice_continue(int wait);

int engine_stop_clear();
int engine_continue();
int engine_add_word(char * str);

#endif            //__VOICE
