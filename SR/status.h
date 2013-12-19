#ifndef       __STATUS
#define       __STATUS

#define       ID_LENGTH      10
#define       STATUS_TOTAL   30

typedef   struct{
  char * ID;
  char * Entry;
}Status,*p_Status;

#define    ENTRY_IGNORE             "__Ignore"
#define    MP3_STOP                 "__MP3_STOP"

#define    SPEECH_NOMATCH           "__NOMATCH"

#define    CONFIG_FILE              "machine.xml"

#define    INITIAL_NODE             "initial"

#define    TYPE_STR_AUTO            "Auto"
#define    TYPE_STR_VOICE           "Voice"

enum {EV_AUTO,EV_SPEECH,EV_KEY,EV_VOLTAGE};

#define       VALUE_LENGTH   30
#define       EVENT_TOTAL    30

typedef   struct{
  int   type;
  int   target;
  char  * cond;
}Event,*p_Event;

typedef   struct{
  Status   list[STATUS_TOTAL];
  int cur,child,end;
}Status_Machine,* p_Status_Machine;

typedef   struct{
  Event    list[EVENT_TOTAL];
  int end;
}Event_List , *p_Event_List;

#endif        //__STATUS
