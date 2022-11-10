#pragma once

#include <curses.h>
#include <unistd.h>

#include <fstream>
#include <iterator>
#include <map>
#include <unordered_map>
#include <vector>

#define NOMAL_M 0
#define INSERT_M 1
#define COMMAND_M 2

#define NOMAL 1
#define MACRO 2
#define TYPE 3
#define NUMBER 4
#define PARENTHESES 5
#define BRACKETS 6
#define VARIABLE 7
#define RESERVED 8
#define OP 9
#define COMMANDLINE 10
#define STATUS 11
#define SUBWIN 12
#define SP_RESERVED 13
#define COMMENT 14
#define FUNCTION 15
#define DARK_BLUE 16
#define RIGHT_GREANANDBLUE 17
#define NOMAL_COLOR 18
#define INSERT_COLOR 19
#define COMMAND_COLOR 20
#define NOMAL_MODE 21
#define INSERT_MODE 22
#define COMMAND_MODE 23
#define LINE 24
#define VISUAL 0x8

using namespace std;

enum {
    kESC = 27,
    kBS = 8,
    kDEL = 127,
    kCtrlSpace = 0,
    kCtrlD = 4,
    kCtrlU = 21,
    kEnter = 13,
    kCtrlN = 14,
    kCtrlP = 16,
    kCtrlQ = 17,
    kCtrlS = 19,
    kCtrlR = 18,
    kCtrlF = 6,
    VisualMode = 3,
};

typedef struct {
    string word;
    int type;
} Token;

extern bool split_token(string::iterator data, const char *word, int index);

extern void tokenPaint(int *nowToken, int *tokenCounter, const int len,
                       const int attribute);
extern vector<Token> initPredictiveTransform();
extern void backChange();

extern void display();
extern void redraw();
extern void quit();

extern void insertMode();
extern void visualMode();
extern void del();

extern int gIndex;
extern int gLines;
extern vector<char> gBuf, gUndoBuf;
extern vector<string> finderData;
extern int gPageStart, gPageEnd;
extern int gCol, gRow;
extern int LineStart, LineEnd;
extern int nowLineNum;
extern int w, h;
extern int nowLineBuf;
extern int nowMode;
extern string commandLineWord;
extern bool gDone;
extern bool finderSwitch;
extern const char *gFileName;

