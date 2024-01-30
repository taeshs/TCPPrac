
typedef enum class CMDCODE {
	CMD_ECHO = 0,
	CMD_CHAT = 100,
	CMD_ENTERROOM = 200,
	CMD_LEAVEROOM,
}CMDCODE;

typedef struct MYCMD {
	CMDCODE nCode;
}MYCMD;

typedef enum class THREADSTATUS {
	TH_NO_ROOM = 1001,
	TH_ROOM_IN,
}THS;

typedef enum class FUNC_RETURN {
	FAIL,
	SUCCESS,
	SUCCESS_EMPTY,
	F_ERROR,
}FR;


#define MAX_ROOM_PLAYER 3