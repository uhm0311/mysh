#include "my_headers.h"
#include "my_variables.h"

bool get_line(char *line, int line_size);
char linux_getch();
void gotoxy(int x, int y);
int get_current_line_number();
void clear_line(int current_line_size);
void close_and_dup(int closer, int duper);

int run(char *line);
void forking_CMD_simple(char *line, bool background);
void forking_CMD_complex(char *line, bool background, int pipes, int redirections);
void exe_in_child(char *tokens[]);
int check_color(char *tokens[]);

void check_backgrounds(bool foreground);
int tokenize(char *buf, char *delims, char *tokens[]);

int main() {
  char line[LINE_SIZE];
  int valid_cmd = 0;
  int i = 0;
  
  back_children = (int *)malloc(BACKGROUNDS * sizeof(int));
  
  for (i = 0; i < BACKGROUNDS; i++) {
    back_children[i] = -1;
    back_commands[i] = (char *)malloc(LINE_SIZE * sizeof(char *));
    back_commands[i][0] = '\0';
  }

  for (i = 0; i < PREVIOUSES; i++) {
    previous_commands[i] = (char *)malloc(LINE_SIZE * sizeof(char *));
    previous_commands[i][0] = '\0';
  }

  i = 0;
  system("clear");

  while (true) {
    char *current_dir = get_current_dir_name();
    printf("%s $ ", current_dir);
    free(current_dir);

    get_line(line, LINE_SIZE);
    if (!(valid_cmd = run(line))) {
      free(back_children);
      for (i = 0; i < BACKGROUNDS; i++) {
        free(back_commands[i]);
      }
      for (i = 0; i < PREVIOUSES; i++) {
        free(previous_commands[i]);
      }
      return 0;
    } else if (valid_cmd != NOT_CMD) {
      if (i >= PREVIOUSES) { //원형으로 쓴다.
        i = 0;
      }
      if (current_previouses < PREVIOUSES) {
        current_previouses++;
      }
      else if (current_previouses >= PREVIOUSES) { //previous_full
        previous_CMD_end = (i + 1);
        if (previous_CMD_end >= PREVIOUSES) {
          previous_CMD_end = 0;
        }

        strcpy(previous_commands[i], line);
        previous_CMD_start = i;

        i++;
      }
    }
  }
}

void clear_line(int current_line_size) {
  int i = current_line_size;
  int j = 0;

  char *current_dir;
  int back_enter = 0;
  int terminal_line_size = 0;

  int stdin_copy;
  struct winsize ws; //터미널 크기 구하기.

  current_dir = get_current_dir_name();
  j = strlen(current_dir);
  ioctl(fileno(stdin), TIOCGWINSZ, &ws);

  terminal_line_size = (j + i + 3); //3은 &출력과 i와 for문에 의한 영향을 모두 포함
  for (back_enter = 0; terminal_line_size > 0; terminal_line_size -= ws.ws_col) {
    back_enter++;
  }

  back_enter--; //for문의 영향으로 1을 뺌

  gotoxy(0, get_current_line_number() - back_enter);
  gotoxy(j + 4, get_current_line_number()); //j + 4는 쉘 띄운 부분. 4는 $출력과 for문에 의한 영향을 모두 포함

  for (j = 0; j < i; j++) {
    printf(" "); //공백 출력해서 지우는 효과
  }

  gotoxy(0, get_current_line_number() - back_enter);
  printf("%s $ ", current_dir);
  free(current_dir);
}

bool get_line(char *line, int line_size) {
  char ch;
  bool del;

  int i = 0;
  int j = 0;

  int arrow_flag = 0;
  bool no_more_up = false;
  bool no_more_down = false;

  int up_down_factor = 0;
  int up_down_subtracter = 0;

  int ups = 0; //화살표 위로 몇번이나 눌렀는지

  int index = -1;
  char *current_dir;

  memset(line, 0, line_size);
  i = 0;

  while (true) {
    if (i >= (line_size - 1)) {
      break;
    }

    ch = linux_getch();
    if (ch == 27) {
      ch = linux_getch();
      if (ch == 91) {
        ch = linux_getch();
        switch (ch) {
          case 65:
            arrow_flag = ARROW_UP;
            break;
          case 68:
            arrow_flag = ARROW_LEFT;
            break;
          case 67:
            arrow_flag = ARROW_RIGHT;
            break;
          case 66:
            arrow_flag = ARROW_DOWN;
            break;
          default:
            arrow_flag = 0;
            if (ch == 51) {
              ch = linux_getch();
              if (ch == 126) {
                del = true;
              } else {
                del = false;
              }
            }
            break;
        }
      } else {
        arrow_flag = 0;
        del = false;
      }
    } else {
      arrow_flag = 0;
      del = false;
    }

    if (arrow_flag) {
      switch (arrow_flag) {
        case 0:
          break;
        case ARROW_LEFT:
        case ARROW_RIGHT:
          break; //좌우는 커서이동 이후의 후속 처리가 까다로워서 처리를 넘긴다.
        default:
          clear_line(i);

          if (arrow_flag == ARROW_UP) {
            up_down_factor = 0;
            up_down_subtracter = 1;
          } else if (arrow_flag == ARROW_DOWN) {
            up_down_factor = 2;
            up_down_subtracter = -1;
          }

          index = previous_CMD_start - (ups - up_down_factor);
          if (previous_CMD_end != 0 && index < 0) {
            index = PREVIOUSES - index;
          }

          if ((previous_CMD_end == 0) && (index >= 0 && index <= current_previouses - 1) || (index >= 0 && index <= previous_CMD_end))
          {
            memset(line, 0, line_size);
            strcpy(line, previous_commands[previous_CMD_start - (ups - up_down_factor)]);
            j = strlen(line);

            if (j <= 0) {
              i = 0;
            } else {
              i = j; //글자수
            }
            
            if (arrow_flag == ARROW_UP) {
              no_more_up = false;
              ups++;
            } else if (arrow_flag == ARROW_DOWN) {
              no_more_up = false;
              ups--;
            }
          } else {
            j = strlen(previous_commands[previous_CMD_start  - (ups - up_down_factor - up_down_subtracter)]);
            
            if (j <= 0) {
              i = 0;
            } else {
              i = j;
            }

            strcpy(line, "");
            clear_line(i - 1);

            if (arrow_flag == ARROW_UP && !no_more_up) {
              no_more_up = true;
              ups++;
            } else if (arrow_flag == ARROW_DOWN && !no_more_down) {
              no_more_down = true;
              ups--;
            }
          }
          for (j = 0; line[j]; j++) {
            if (line[j] == '\r' || line[j] == '\n') {
              break;
            }
            putchar(line[j]);
          }

          break;
      }
    } else {
      if (!del && ch != 127) { //딜리트키와 화살표는 까다로워서 처리를 넘긴다.
        line[i++] = ch;
        putchar(ch);
      } else if (ch == 127) { //백스페이스이면
        if (i > 0) {
          putchar(ch);
          line[i--] = '\0';
        }
      }
    }

    if (ch == '\r' || ch == '\n') {
      line[i] = '\0';
      break;
    }
  }

  return true;
}

char linux_getch() {
  struct termios oldt, newt;
  int ch;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~ (ICANON | ECHO);

  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

  return ch;
}

void gotoxy(int x, int y) {
  printf("\033[%d;%df", y, x);
  fflush(stdout);
}

int get_current_line_number() {
  char buf[8];
  char line_number[3];
  char cmd[] = "\033[6n";

  struct termios save, raw;

  tcgetattr(0, &save);
  cfmakeraw(&raw);
  tcsetattr(0, TCSANOW, &raw);

  if (isatty(fileno(stdin))) {
    write(1, cmd, sizeof(cmd));
    read(0, buf, sizeof(buf));

    line_number[0] = buf[2];
    line_number[1] = buf[3];
    line_number[2] = '\0';
  }
  tcsetattr(0, TCSANOW, &save);

  return atoi(line_number);
}

int run(char *line)
{
  char *line0 = NULL;
  char *line_no_background = NULL;

  char *delims0 = " \t\r\n";
  char *tokens0[TOKEN_SIZE];

  int token_count0 = 0;
  int i, j, k;

  int pipes = 0;
  int redirections = 0;
  bool background = false;

  line0 = (char *)malloc(LINE_SIZE);
  line_no_background = (char *)malloc(LINE_SIZE);

  for (i = 0; i < TOKEN_SIZE; i++) {
    tokens0[i] = NULL;
  }

  for (i = 0, j = 0; line[i]; i++) {
    if (line[i] == '&') {
      j++;
      background = true;

      continue;
    } else if (line[i] == '|') {
      for (k = 0; k < i; k++) {
        if (line[k] == '>') {
          free(line0);
          free(line_no_background);

          fprintf(stderr, "redirection before pipe is not permitted.\r\n");
          return true;
        } else if (line[k] == '&') {
          free(line0);
          free(line_no_background);

          fprintf(stderr, "syntax error near unexpected token \'&\'\r\n");
          return true;
        }
      }

      pipes++;
    } else if (line[i] == '>') {
      for (k = 0; k < i; k++) {
        if (line[k] == '&') {
          free(line0);
          free(line_no_background);

          fprintf(stderr, "syntax error near unexpected token \'&\'\r\n");
          return true;
        }
      }
      redirections++;
    }

    line0[i - j] = line[i];
  }

  if (j > 1) {
    free(line0);
    free(line_no_background);

    fprintf(stderr, "syntax error near unexpected token \'&\'\r\n");
    return true;
  }

  line0[i - j] - '\0';
  strcpy(line_no_background, line0);
  token_count0 = tokenize(line0, delims0, tokens0);

  if (token_count0 < 2) {
    check_backgrounds(true);
    return NOT_CMD;
  } else {
    if (!strcmp(tokens0[0], "exit") || !strcmp(tokens0[0], "quit")) {
      return false;
    } else if (!strcmp(tokens0[0], "cd")) {
      if (token_count0 == 2) {
        chdir(getenv("HOME"));
      } else {
        chdir(tokens0[1]);
      }
    } else if (!strcmp(tokens0[0], "help") || !strcmp(tokens0[0], "?")) {
      fprintf(stdout, "\t\tSimple Shell\r\n");
      fprintf(stdout, "You can use it just as the convectional shell.\r\n\r\n");

      fprintf(stdout, "Some examples of the built-in commands:\r\n");
      fprintf(stdout, "cd\t: change directory.\r\n");
      fprintf(stdout, "exit\t: exit this shell.\r\n");
      fprintf(stdout, "quit\t: quit this shell.\r\n");
      fprintf(stdout, "help\t: show this help.\r\n");
      fprintf(stdout, "?\t: show this help.\r\n");
    } else {
      if (!redirections && !pipes) {
        forking_CMD_simple(line_no_background, background);
      } else {
        forking_CMD_complex(line_no_background, background, pipes, redirections);
      }
    }
  }

  free(line0);
  free(line_no_background);

  return true;
}

void exe_in_child(char *tokens[]) {
  if (execvp(tokens[0], tokens)) {
    fprintf(stderr, "execute \'%s\' : %s\r\n", tokens[0], strerror(errno));
    exit(-1 * errno);
  }
  fprintf(stderr, "execvp : \r\n%s\r\n", strerror(errno));
  exit(-1);
}

int check_color(char *tokens[]) {
  int i = 0;

  if (!strcmp(tokens[0], "ls") || !strcmp(tokens[0], "grep")) {
    for (i = 0; tokens[i]; i++) {
      if (strstr(tokens[i], "--color")) {
        return -1;
      }

      tokens[i] = (char *)malloc(13);
      strcpy(tokens[i], "--color=auto");

      return i;
    }
  }

  return -1;
}

void forking_CMD_simple(char *line, bool background) {
  char *delims = " \t\r\n";
  char *tokens[TOKEN_SIZE];

  int token_count = 0;
  int child = 0;

  int i = 0;
  int color_CMD_index = 0;

  for (i = 0; i < TOKEN_SIZE; i++) {
    tokens[i] = NULL;
  }

  token_count = tokenize(line, delims, tokens);
  color_CMD_index = check_color(tokens);

  if (!(child = fork())) {
    exe_in_child(tokens);
  }

  if (background) {
    fprintf(stdout, "[%d] %d\r\n", backs + 1, child);

    while (back_children[backs] != -1) {
      backs++;
    }

    back_children[backs] = child;

    for (i = 0; tokens[i]; i++) {
      strcat(back_commands[backs], tokens[i]);
      strcat(back_commands[backs], " ");
    }

    backs++;
  } else {
    wait();
    usleep(USLEEP_TIME);
  }

  check_backgrounds(!background);

  if (color_CMD_index >= 0) {
    free(tokens[color_CMD_index]);
    color_CMD_index = -1;
  }
}

void forking_CMD_complex(char *line, bool background, int pipes, int redirections) {
  char *line1 = NULL;
  char *line2 = NULL;
  char *line3 = NULL;

  char *delims1 = "|>";
  char *delims2 = " \t\r\n";
  char *delims3 = NULL;

  char *tokens1[TOKEN_SIZE];
  char *tokens2[TOKEN_SIZE][TOKEN_SIZE];
  char *tokens3[TOKEN_SIZE];

  int token_count1 = 0;
  int *token_count2 = NULL;
  int token_count3 = 0;

  int child = 0;
  int i, j, k;

  int pipe_fd[pipes][2];
  int *redirect_fd = NULL;
  int redirect_file_factor = 0;

  int *color_CMD_index = NULL;
  int stdout_copy = 0;

  char *current_dir;

  line1 = (char *)malloc(LINE_SIZE);
  line2 = (char *)malloc(LINE_SIZE);
  line3 = (char *)malloc(LINE_SIZE);

  strcpy(line1, line);
  strcpy(line2, line);
  strcpy(line3, line);

  for (i = 0; i < TOKEN_SIZE; i++) {
    tokens1[i] = NULL;
    tokens3[i] = NULL;

    for (j = 0; j < TOKEN_SIZE; j++) {
      tokens2[i][j] = NULL;
    }
  }

  token_count1 = tokenize(line1, delims1, tokens1);
  token_count2 = (int *)malloc((token_count1 - 1) * sizeof(int));

  delims3 = (char *)malloc(LINE_SIZE);
  color_CMD_index = (int *)malloc((token_count1 - 1) * sizeof(int));

  for (i = 0; i < token_count1 - 1; i++) {
    strcat(delims3, tokens1[i]);

    token_count2[i] = tokenize(tokens1[i], delims2, tokens2[i]);
    color_CMD_index[i] = check_color(tokens2[i]);
  }

  token_count1 = tokenize(line3, delims3, tokens3);

  /*
  ls| grep c> 1.txt>2.txt를 예로 들면
  tokens1은 재토큰화 전을 기준으로
  tokens1[0] = ls
  tokens1[1] = grep c
  tokens1[2] = 1.txt
  tokens1[3] = 2.txt
  tokens1[4] = NULL

  tokens2[0][0] = ls
  tokens2[0][1] = --color=auto
  tokens2[0][2] = NULL
  tokens2[1][0] = grep
  tokens2[1][1] = c
  tokens2[1][2] = --color=auto
  tokens2[1][3] = NULL
  tokens2[2][0] = 1.txt
  tokens2[2][1] = NULL
  tokens2[3][0] = 2.txt
  tokens2[3][1] = NULL

  delims3 = ls grep c 1.txt2.txt

  tokens3[0] = |
  tokens3[1] = >
  tokens3[2] = >
  tokens3[3] = NULL
  */

  if (redirections) {
    redirect_fd = (int *)malloc(redirections * sizeof(int));

    for (i = 0; i < redirections; i++) {
      redirect_fd[i] = open(tokens2[i + (pipes + 1)][0], O_WRONLY | O_CREAT | O_TRUNC, 0664);

      if (redirect_fd[i] < 0) {
        for (j = 0; j < i; j++) {
          close(redirect_fd[j]);
        }

        free(line1);
        free(line2);
        free(line3);

        free(delims3);
        free(token_count2);

        free(redirect_fd);
        free(color_CMD_index);

        fprintf(stderr, "redirections : %s\r\n", strerror(errno));
        return;
      }
    }
  }

  if (pipes) {
    for (i = 0; i < pipes; j++) {
      if (pipe2(pipe_fd[i], O_NONBLOCK)) {
        for (j = 0; j < redirections; j++) {
          close(redirect_fd[j]);
        }

        free(line1);
        free(line2);
        free(line3);

        free(delims3);
        free(token_count2);

        free(redirect_fd);
        free(color_CMD_index);

        for (j = 0; j <= i; j++) {
          close(pipe_fd[j][0]);
          close(pipe_fd[j][1]);
        }
      } else {
        stdout_copy = dup(STDOUT_FILENO);
        close(STDOUT_FILENO);
        fprintf(stdout, "%d %d\r\n", pipe_fd[pipes - 1][0], pipe_fd[pipes - 1][1]); //안하면 알수없는 이유로 pipe_fd[0] 값이 0이 됨
        dup2(stdout_copy, STDOUT_FILENO);
        close(stdout_copy);
      }
    }
  }

  for (i = 0; i < token_count1 - 1; i++) {
    if (!pipes) { //파이프 없음, 리다이렉션 있음
      if (!(child = fork())) {
        close_and_dup(STDOUT_FILENO, redirect_fd[redirections - 1]);
        exe_in_child(tokens2[i]);
      }

      if (background) {
        fprintf(stdout, "[%d] %d\r\n", backs + 1, child);
        while (back_children[backs] != -1) {
          backs++;
        }
        back_children[backs] = child;

        for (j = 0; j < token_count1 - 1; j++) {
          for (k = 0; tokens2[j][k]; k++) {
            strcat(back_commands[backs], tokens2[j][k]);
            strcat(back_commands[backs], " ");
          }

          if (tokens3[j]) {
            strcat(back_commands[backs], tokens3[j]);
          }
        }

        backs++;
      } else {
        wait();
        usleep(USLEEP_TIME);
      }

      check_backgrounds(!background);
      break; //리다이렉션까지 모두 마쳤으므로
    } else if (pipes) {
      if (!i) { //파이프 왼쪽. 리다이렉션의 유무는 영향 없음
        if (!(child = fork())) {
          close_and_dup(STDOUT_FILENO, pipe_fd[i][1]);
          exe_in_child(tokens2[i]);
        }
      } else if (i < pipes) { //파이프 중간. 리다이렉션 유무는 영향 없음
        if (!(child = fork())) {
          close_and_dup(STDIN_FILENO, pipe_fd[i - 1][0]);
          close_and_dup(STDOUT_FILENO, pipe_fd[i][1]);

          exe_in_child(tokens2[i]);
        }
      } else if (i == pipes) { //파이프 오른쪽
        if (!(child = fork())) {
          close_and_dup(STDIN_FILENO, pipe_fd[i - 1][0]);
          if (redirections) {
            close_and_dup(STDOUT_FILENO, redirect_fd[redirections - 1]);
          }
          exe_in_child(tokens2[i]);
        }
      }

      if ((i == pipes) && background) {
        fprintf(stdout, "[%d] %d\r\n", backs + 1, child);
        while (back_children[backs] != -1) {
          backs++;
        }
        back_children[backs] = child;

        for (j = 0; j < token_count1; j++) {
          for (k = 0; tokens2[j][k]; k++) {
            strcat(back_commands[backs], tokens2[j][k]);
            strcat(back_commands[backs], " ");
          }

          if (tokens3[j]) {
            strcat(back_commands[backs], tokens3[j]);
          }
        }

        backs++;
      } else {
        wait();
        usleep(USLEEP_TIME);
      }

      check_backgrounds(!background);

      if ((i == pipes) && redirections) {
        break; //리다이렉션까지 모두 마쳤으므로
      }
    }
  }

  free(line1);
  free(line2);
  free(line3);

  free(delims3);
  free(token_count2);

  if (redirections) {
    for (i = 0; i < redirections; i++) {
      close(redirect_fd[i]);
    }
    free(redirect_fd);
  }

  if (color_CMD_index) {
    for (i = 0; i < token_count1; i++) {
      if (color_CMD_index[i] >= 0) {
        free(tokens2[i][color_CMD_index[i]]);
      }

      free(color_CMD_index);
    }
  }

  for (i = 0; i < pipes; i++) {
    close(pipe_fd[i][0]);
    close(pipe_fd[i][1]);
  }
}

void check_backgrounds(bool foreground) {
  int i = 0;
  int exit_flag = 0;

  for (i = 0; i < BACKGROUNDS; i++) {
    if (back_children[i] > 0) {
      waitpid(-1, &exit_flag, WNOHANG);

      if (kill(back_children[i], 0) == -1) {
        back_children[i] = -1;

        if (foreground) {
          fprintf(stdout, "[%d]+\t", (i + 1));
          backs = 0;

          while (back_children[backs] != -1) {
            backs++;
          }
        } else {
          fprintf(stdout, "[%d]\t", (i + 1));
        }

        if (exit_flag) {
          fprintf(stdout, "Exit %d\t\t\t", exit_flag);
        } else {
          fprintf(stdout, "Done\t\t\t");
        }

        fprintf(stdout, "%s\r\n", back_commands[i]);
        back_commands[i][0] = '\0';
      }
    }
  }
}

int tokenize(char *buf, char *delims, char *tokens[]) {
  char *token;
  int token_count = 0;

  token = strtok(buf, delims);

  while ((token != NULL) && (token_count < TOKEN_SIZE - 1)) {
    tokens[token_count++] = token;
    token = strtok(NULL, delims); //첫번째 인자를 NULL로 하면 기존 포인터에서 계속 자르게 된다.
  }

  tokens[token_count++] = NULL;
  return token_count;
}

void close_and_dup(int closer, int duper) {
  if (close(closer)) {
    if (closer) {
      fprintf(stderr, "close stdout : %s\r\n", strerror(errno));
    } else {
      fprintf(stderr, "close stdin : %s\r\n", strerror(errno));
    }
    exit(errno * (-1));
  }

  if (dup2(duper, closer) < 0) {
    fprintf(stderr, "dup %d : %s after close %d\r\n", duper, strerror(errno), closer);
    exit(errno * (-1));
  }
}