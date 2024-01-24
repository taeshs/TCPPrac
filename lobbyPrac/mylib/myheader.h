
typedef enum CMDCODE {
	CMD_ECHO = 0,
	CMD_CHAT = 100,
	CMD_ENTERROOM = 200,
	CMD_LEAVEROOM,
}CMDCODE;

typedef struct MYCMD {
	int nCode;
	
}MYCMD;