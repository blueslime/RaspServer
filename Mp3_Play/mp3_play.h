//////////////////////////////////////////////////////////////////////////
// Modified by zwj
// zwj_uptech@126.com
// http://robot.up-tech.com
//////////////////////////////////////////////////////////////////////////
#ifndef           __MP3_PLAYER
#define           __MP3_PLAYER

int play(const char * path);
int stop_play(int wait);
int player_init();
int machine_exit();
void *mp3_loop(void *arg);
void put_res_player();
int isMP3Playing();

#endif            //__MP3_PLAYER
