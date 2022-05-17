#ifndef _TEXTEDITOR_H
#define _TEXTEDITOR_H

// includes
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>

// defines
#define CTRL_KEY(k) ((k) & 0x1f)
#define BUF_INIT {NULL, 0}
#define VERSION "1.0.0"
#define TAB_SPACE 4
#define QUIT_TIMES 1

enum keys {
    BACKSPACE = 127,
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN
};

// data
typedef struct erow {
    int size;
    int renderSize;
    char *str;
    char *render;
} erow;

struct editorConfig {
    int cursorX;
    int cursorY;
    int renderX;
    int rowOffset;
    int colOffset;
    int rows;
    int cols;
    int numrows;
    int dirty;
    char *filename;
    char statusMsg[80];
    time_t statusMsgTime;
    erow *currentRow;
    struct termios origTermios;
};

struct buf {
    char *str;
    int len;
};

struct editorConfig config;

//method declarations
void enableRawMode();
void disableRawMode();
void initEditor();
void processKeyPress();
int readKeyPress();
void moveCursor(int key);
char *prompt(char *prompt, void (callback)(char *, int));
void openFile(char *filename);
char *rowsToString(int *bufferLen);
void save();
void find();
void findCallback(char *query, int key);
int getWindowSize(int *rows, int *cols);
int getCursorPosition(int *rows, int *cols);
void bufAppend(struct buf *b, const char *s, int len);
void bufFree(struct buf *b);
void insertRow(int at, char *str, size_t len);
void updateRow(erow *row);
int cursorXToRenderX(erow *row, int cursorX);
int renderXToCursorX(erow *row, int renderX);
void rowInsertChar(erow *row, int at, int c);
void rowDeleteChar(erow *row, int at);
void freeRow(erow *row);
void deleteRow(int at);
void rowAppendString(erow *row, char *str, size_t len);
void insertChar(int c);
void deleteChar();
void insertNewLine();
void rewriteScreen();
void writeRow(struct buf *b);
void scroll();
void writeStatus(struct buf *b);
void setStatusMessage(const char *str, ...);
void writeMessageBar(struct buf *b);
void die(const char *str);

#endif