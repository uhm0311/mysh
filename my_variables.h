#ifndef MY_VARIABLES
#define MY_VARIABLES 0

#define PREVIOUSES 64
#define BACKGROUNDS 256
#define TOKEN_SIZE 512
#define LINE_SIZE 1024

#define ARROW_LEFT 1
#define ARROW_RIGHT 2
#define ARROW_UP 3
#define ARROW_DOWN 4

#define USLEEP_TIME 100 //50000

#define NOT_CMD 2 //공백같은걸 입력한 경우

int *back_children;
char *back_commands[BACKGROUNDS];
int backs;

char *previous_commands[PREVIOUSES]; //이건 FILO으로 작동된다.
int previous_CMD_start = 0;
int previous_CMD_end = 0;
int current_previouses = 0;

#endif