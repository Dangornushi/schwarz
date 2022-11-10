#include "schwarz.hpp"

int lineTop(const int inOffset) {
    int offset = inOffset - 1;
    while (offset >= 0 && gBuf[offset] != '\n')
        offset--;
    return offset <= 0 ? 0 : ++offset;
}

int nextLineTop(const int inOffset) {
    int offset = inOffset;
    while (offset < gBuf.size() && gBuf[offset++] != '\n'); /* empty */
    return offset < gBuf.size() ? offset : gBuf.size() - 1;
}

int adjust(const int inOffset, const int inCol) {
    int offset = inOffset;
    for (int i = 0; offset < gBuf.size() && gBuf[offset] != '\n' && i < inCol;offset++)
        i += gBuf[offset] == '\t' ? 4 - (i & 3): 1;

    return offset;
}

// minimal Move Commands
void left()      { if (gBuf[gIndex-1] != '\n')--gIndex;}
void right()     { if (gBuf[gIndex] != '\n')++gIndex;}
void up()        { gIndex = adjust(lineTop(lineTop(gIndex) - 1), gCol); (nowLineNum > 1) ? nowLineNum-- : 1;}// nowlineNum-- <- 行数を一つマイナス
void down()      { gIndex = adjust(nextLineTop(gIndex), gCol); (gIndex < gBuf.size()-1) ? nowLineNum++ : 1;} // nowlineNum++ <- 行数を一つ追加
void gotoUp()    { gIndex = adjust(gPageStart, 0); }
void gotoDown()  { gIndex = adjust(gPageEnd-1, 0); }
void lineBegin() { gIndex = lineTop(gIndex); }
void lineEnd()   { while (gBuf[gIndex] != '\n') gIndex++;}// nextLineTop(gIndex);}
void top()       { gIndex = 0; }
void bottom()    { gIndex = gBuf.size() - 1; }

void del()       { if (gIndex < gBuf.size() - 1) gBuf.erase(gBuf.begin() + gIndex);}
void quit()      { gDone = true; }
void redraw()    { clear(); display(); }

void wordLeft() {
    while (!isspace(gBuf[gIndex]) && 0 < gIndex)
        --gIndex;

    while (isspace(gBuf[gIndex]) && 0 < gIndex)
        --gIndex;
}

void wordRight() {
    while (!isspace(gBuf[gIndex]) && gIndex < gBuf.size())
        ++gIndex;
    while (isspace(gBuf[gIndex]) && gIndex < gBuf.size())
        ++gIndex;
}

void pageDown() {
    gPageStart = gIndex = lineTop(gPageEnd - 1);
    while (0 < gRow--)
        down();

    gPageEnd = gBuf.size() - 1;
}

void pageUp() {
    for (int i = LINES; 0 < --i; up())
        gPageStart = lineTop(gPageStart - 1);
}

void oneWordMove() {
    while (gBuf[gIndex+2] != ' ' || gBuf[gIndex+2] != '\t' || gBuf[gIndex+2] != '\n') 
        gIndex++;
}

void oneWordBack() {
    while (gBuf[gIndex-1] != ' ' || gBuf[gIndex-1] != '\t' || gBuf[gIndex-1] != '\n')
        gIndex--;
}

void gotoLine() {
    char ch[100];
    char input;

    getstr(ch);

    string commandLineNumber = ch;
    int wantToGoLine = stoi(commandLineNumber);

    gIndex = 0;

    commandLineWord = "go to: Line " + commandLineNumber;
    redraw();

    if ((input = getch()) != '\n') {
        commandLineWord = "! Canceled !";
        redraw();
        return;
    }

    for (int followLine = 1; followLine < wantToGoLine; gIndex++)
        (gBuf[gIndex] == '\n') ? followLine++ : 0;

    commandLineWord = "jump to: Line " + commandLineNumber;
    redraw();
}

