// main.cpp   Mushi's Editor 2022.
// compile: g++ -std=c++1z ae2019.cpp -lcurses
// Github... Comming soon

#include <curses.h>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <unistd.h>

#define BACK        0
#define NOMAL       1
#define MACRO       2
#define TYPE        3
#define NUMBER      4
#define PARENTHESES 5
#define BRACKETS    6
#define VARIABLE    7

using namespace std;

enum {
    kESC = 27,
    kBS = 8,
    kDEL = 127,
    kCtrlD = 4,
    kCtrlU = 21,
    kCtrlR = 18,
    kCtrlS = 19,
    kCtrlQ = 17,
    kEnter = 13,
};

enum {
    VisualMode = 3,
};

const char *gFileName;
vector<char> gBuf;
string commandLineWord;
string yankBuf;
bool gDone = false;
int gIndex = 0 /* offset of cursor pos */, gPageStart = 0, gPageEnd = 0;
int gCol, gRow;
int w, h;
int nowMode, nowLineNum = 1, renderingLineNum = 1, colorSet = 1;

bool  split_token(string::iterator data, const char *word, int index) {
     for (int i = 0; i < index; i++)
        if (data[i] != word[i])
            return false;
    return true;
}

int lineTop(const int inOffset) {
    int offset = inOffset - 1;
    while (offset >= 0 && gBuf[offset] != '\n') {
        offset--;
    }
    return offset <= 0 ? 0 : ++offset;
}

int nextLineTop(const int inOffset) {
    int offset = inOffset;
    while (offset < gBuf.size() && gBuf[offset++] != '\n') { /* empty */ }
    return offset < gBuf.size() ? offset : gBuf.size() - 1;
}

int adjust(const int inOffset, const int inCol) {
    int offset = inOffset;
    for (int i = 0; offset < gBuf.size() && gBuf[offset] != '\n' && i < inCol;offset++) {
        i += gBuf[offset] == '\t' ? 4 - (i & 3) : 1;
    }
    return offset;
}

auto putColor(std::__wrap_iter<char*>  p) {
    return p;
}

void display() {
    int howChangeStart = 0, howManyEnter = 0;

    // 上にスクロール
    if (gIndex <= gPageStart) {
        gPageStart = lineTop(gIndex);
    }

    // 下の行のデータを取得
    if (gPageEnd <= gIndex) {
        gPageStart = nextLineTop(gIndex); // 次のラインのインデックスを取得 
        int n = LINES-1; // ファイル容量 - コマンド表示欄分マイナス 

        for (int i = 0; i < n; i++) {
            gPageStart = lineTop(gPageStart - 1);
        }
    }

    move(0, 0);
    int i = 0, j = 0;
    gPageEnd = gPageStart;


    for (auto p = gBuf.begin() + gPageEnd; /*empty */; gPageEnd++, p++) {
        if (gIndex == gPageEnd) {
            // update cursor pos.
            gRow = i;
            gCol = j;
        } 
        if (LINES-1 <= i || gBuf.size() <= gPageEnd)
            break;
        if (*p != '\r') {
            /*
            // if colour options are set
            switch(*p) {
                case '#': {
                    for (; *p != ' '; j++) {
                        attrset(COLOR_PAIR(MACRO));
                        addch(*p++);
                        gPageEnd++;
                    }
                    p--;
                    break;
                }
                case '(':
                case ')': {
                    attrset(COLOR_PAIR(PARENTHESES));
                    addch(*p);
                    break;
                }
                case '{':
                case '}': {
                    attrset(COLOR_PAIR(BRACKETS));
                    addch(*p);
                    break;
                }
                default: {
                    if (split_token(p, "int", 3)) {
                        attrset(COLOR_PAIR(TYPE));
                        for (int i = 0; i < 3; i++, j++) {
                            addch(*p++);
                            gPageEnd++;
                        }
                        p--;
                        gPageEnd--;
                        j--;
                    } else {
                             */
                        attrset(COLOR_PAIR(NOMAL));
                        addch(*p);
                        /*
                    }
                }
            }
                         */
            j += *p == '\t' ? 4 - (j & 3) : 1;
        }
        if (*p == '\n' || COLS <= j) {
            ++i;
            j = 0;
        }
    }

    attrset(COLOR_PAIR(NOMAL));
    for (;i < h-1;i++)
        mvaddstr(i, 0, "~");

    if (gRow < LINES-1)
        gRow += howChangeStart;

    string fileName = gFileName;
    string cursorRow = "| " + fileName + " | " + to_string(nowLineNum) + "*" + to_string(gCol+1) + " ";
    mvaddstr(i, 0, commandLineWord.c_str());
    mvaddstr(i, COLS - cursorRow.size(), cursorRow.c_str());
    clrtobot();
    move(gRow, gCol);
    refresh();
}

void left()      { if (gBuf[gIndex-1] != '\n' && gIndex > 0) --gIndex;}
void right()     { if (gBuf[gIndex] != '\n') ++gIndex;}
void up()        { gIndex = adjust(lineTop(lineTop(gIndex) - 1), gCol); (nowLineNum > 1) ? nowLineNum-- : 1;}
void down()      { gIndex = adjust(nextLineTop(gIndex), gCol); (gIndex < gBuf.size()-1) ? nowLineNum++ : 1;}
void lineBegin() { gIndex = lineTop(gIndex); }
void lineEnd()   { while (gBuf[gIndex] != '\n') gIndex++;}// nextLineTop(gIndex);}
void top()       { gIndex = 0; }
void bottom()    { gIndex = gBuf.size() - 1; }
void del()       { if (gIndex < gBuf.size() - 1) gBuf.erase(gBuf.begin() + gIndex);}
void quit()      { gDone = true; }

void redraw() {
    clear();
    display();
}

static void insert() {
    commandLineWord = "! insert !";
    for (int ch; (ch = getch()) != kESC; display()) {
        if (gIndex > 0 && (ch == kBS || ch == kDEL)) {
            gBuf.erase(gBuf.begin() + (--gIndex));
            nowLineNum--;
        }
        else if (ch == '\n') {
            gBuf.insert(gBuf.begin() + gIndex++, ch);
            nowLineNum++;
        }
        else
            gBuf.insert(gBuf.begin() + gIndex++, ch == '\r' ? '\n' : ch);
    }
    commandLineWord = "> Command";
}

void wordLeft() {
    while (!isspace(gBuf[gIndex]) && 0 < gIndex) {
        --gIndex;
    }
    while (isspace(gBuf[gIndex]) && 0 < gIndex) {
        --gIndex;
    }
}

void wordRight() {
    while (!isspace(gBuf[gIndex]) && gIndex < gBuf.size()) {
        ++gIndex;
    }
    while (isspace(gBuf[gIndex]) && gIndex < gBuf.size()) {
        ++gIndex;
    }
}
void pageDown() {
    gPageStart = gIndex = lineTop(gPageEnd - 1);
    while (0 < gRow--) {
        down();
    }
    gPageEnd = gBuf.size() - 1;
}
void pageUp() {
    for (int i = LINES; 0 < --i; up()) {
        gPageStart = lineTop(gPageStart - 1);
    }
}

void save() {
    ofstream ofs(gFileName, ios::binary);
    ostream_iterator<char> output_iterator(ofs);
    copy(gBuf.begin(), gBuf.end(), output_iterator);
    commandLineWord = "===--- saved ---===";
    redraw();
}

void gotoUp() {
    gIndex = adjust(gPageStart, 0);
}

void gotoDown() {
    gIndex = adjust(gPageEnd-1, 0);
}

void renderingNowLine() {
    string number = "| " + to_string(nowLineNum) + " : " + to_string(gCol) + " |";
    string rail;

    for (int size=0;size<number.size()-2;size++)
        rail += "-";

    
    savetty();
    mvaddstr(gRow+1, gCol, ("." + rail + ".").c_str());
    mvaddstr(gRow+2, gCol, number.c_str());
    mvaddstr(gRow+3, gCol, ("'" + rail + "'").c_str());
    move(gRow, gCol);
    refresh();
    getch();
    resetty();
}

void commandMode() {
    string before_commandLineWord = commandLineWord;
    commandLineWord = "> ";
    redraw();
    char ch = getch();

    if (ch == 'w')
        save();
    if (ch == 'q')
        quit();
    if (ch == 'l')
        renderingNowLine();

    commandLineWord = before_commandLineWord;
    redraw();
}

void newLine() {
    lineEnd();
    gBuf.insert(gBuf.begin() + gIndex, '\n');
    gIndex++;
    redraw();
    insert();
}

void oneWordMove() {while (gBuf[gIndex+2] != ' ' || gBuf[gIndex+2] != '\t' || gBuf[gIndex+2] != '\n') gIndex++;}
void oneWordBack() {while (gBuf[gIndex-1] != ' ' || gBuf[gIndex-1] != '\t' || gBuf[gIndex-1] != '\n') gIndex--;}

void lineDel() {
    /*
    for (; gBuf[gIndex] != '\n'; gIndex++) {
        ++moveDiff;
        yankBuf.push_back(gBuf[gIndex]);
    }
    for (int i = 0; i < gCol; i++) {
        yankBuf.push_back(gBuf[gIndex++]);
    }
    */
}

void visualMode() {
    nowMode = VisualMode;
    int moveDiff = 0;
    int base_gCol;
    int base_gIndex;

    yankBuf.clear();

    base_gIndex = gIndex;
    base_gCol = gCol;

    for (int ch; (ch = getch()) != kESC; display()) {
        switch (ch) {
            case 'h': {
                (--moveDiff > 0) ? yankBuf.resize(yankBuf.size() - 1)
                                  : yankBuf.push_back(gBuf[gIndex]);
                gIndex--;
                break;
            }
            case 'j': {
                int tmp_gIndex = gIndex;
                for (;gBuf[gIndex] != '\n';gIndex++);
                gIndex++;
                gIndex += gCol;

                for (;base_gIndex+1 < gIndex; ++moveDiff, tmp_gIndex++)
                    yankBuf.push_back(gBuf[gIndex--]);

                gIndex = ++tmp_gIndex;

                redraw();
                break;
            }
            case 'k': {
                for (;gBuf[gIndex] != '\n';gIndex--);
                gIndex--;
                for (;gBuf[gIndex] != '\n';gIndex--);

                gIndex += gCol+1;

                //base_gIndex++;
                moveDiff-=2;
                for (; gIndex+1 < base_gIndex;--moveDiff)
                    yankBuf.push_back(gBuf[--base_gIndex]);

                redraw();
                break;
            }
            case 'l': {
                (++moveDiff >= 0) ? yankBuf.push_back(gBuf[gIndex])
                                  : yankBuf.resize(yankBuf.size() - 1);
                gIndex++;
                break;
            }
            case 'y': {
                yankBuf.push_back(gBuf[gIndex]);
                if (moveDiff <= 0) reverse(yankBuf.begin(), yankBuf.end());
                return;
            }
            case 'd': {

                for (int i = 0; i < yankBuf.size() + 1; i++) {
                    (++moveDiff >= 0) ? gBuf.erase(gBuf.begin() + base_gIndex)
                                      : gBuf.erase(gBuf.begin() + gIndex);
                }

                gCol = base_gCol;

                redraw();
                return;
            }
            case kESC:
                break;
        }
    }
}
rm -f .DS_Store

void gotoLine() {
    char ch[100];
    char input;
    getstr(ch);
    string commandLineNumber = ch;
    int wantToGoLine = stoi(commandLineNumber);
    gIndex = 0;

    commandLineWord = "go to: Line " + commandLineNumber;
    redraw();

    input = getch();

    if (input != '\n') {
        commandLineWord = "! Canceled !";
        redraw();
        return;
    }

    for (int followLine = 1; followLine < wantToGoLine; gIndex++)
        (gBuf[gIndex] == '\n') ? followLine++ : 0;

    commandLineWord = "jump to: Line " + commandLineNumber;
    redraw();
}

void paste() {
    for (auto ch : yankBuf)
        gBuf.insert(gBuf.begin() + gIndex++, ch == '\r' ? '\n' : ch);
}

void addInsert() {
    gIndex++;
    redraw();
    insert();
}

unordered_map<char, void (*)()> gAction = {
    {'h', left},      {'l', right},     {'k', up},          {'j', down},
    {'b', wordLeft},  {'w', wordRight}, {kCtrlD, pageDown}, {kCtrlU, pageUp},
    {'0', lineBegin}, {'$', lineEnd},   {'T', top},         {'G', bottom},
    {'i', insert},    {'x', del},       {kCtrlQ, quit},     {kCtrlR, redraw},
    {kCtrlS, save},   {'o', newLine}, 
    {'H', gotoUp},    {'L', gotoDown},

    {'f', commandMode}, {':', commandMode}, {' ', commandMode},
    {'d', del},       {'c', lineBegin},
    {'w', oneWordMove}, {'b', oneWordBack},
	{'/', gotoLine},  {'v', visualMode}, {'p', paste}, {'a', addInsert},
    
};

int main(int argc, char **argv) {
    if (argc < 2) {
        return 2;
    }
    initscr();
    /*
     * BLUE 0x12
     * sky_blue 0x20
     * sky_green 0x30
     * white 0x99
     */

    // 色設
    start_color();
    init_pair(NOMAL, 0x99, BACK);   // 黒地に白文字
    init_pair(TYPE, 0x33, BACK);   // 黒地に水色文字
    init_pair(MACRO, 0x20, COLOR_BLACK);   // 黒地に青文字
    init_pair(BRACKETS, 0x21, COLOR_BLACK);   // 黒地に黄色文字
    init_pair(PARENTHESES, 0x88, COLOR_BLACK);   // 黒地に黄色文字
    init_pair(VARIABLE, 0x88, COLOR_BLACK);   // 黒地に黄色文字
    init_pair(4, 0x12, COLOR_BLACK);   // 黒地にオレンジ文字
    attrset(COLOR_PAIR(1));
    
    set_tabsize(4);
    raw();
    noecho();
    idlok(stdscr, true);  // init screen.
    getmaxyx(stdscr, h, w);
    gFileName = argv[1];
    ifstream ifs(gFileName, ios::binary);
    gBuf.assign(istreambuf_iterator<char>(ifs), istreambuf_iterator<char>());
    commandLineWord = "> Command";
    while (!gDone) {
        display();
        char ch = getch();
        if (gAction.count(ch) > 0) {
            gAction[ch]();
        }
    }
    endwin();
    return 0;
}
