// main.cpp   mushi's editor 2022.
// compile: g++ -std=c++1z ae2019.cpp -lcurses
// Github; Dangornushi/schwarz

#include <curses.h>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <map>
#include <unistd.h>
#include <iterator>

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
    VisualMode = 3,
};

typedef struct {
    string word;
    int type;
} Token;


static void insertMode();
static void visualMode();
const char *gFileName;

vector<char> gBuf, gUndoBuf;
vector<Token> initPredictiveTransform();
vector<Token> predictive = initPredictiveTransform();
string commandLineWord;
string yankBuf;
bool gDone = false;
int gIndex = 0 /* offset of cursor pos */, gPageStart = 0, gPageEnd = 0;
int gCol, gRow;
int w, h;
int nowMode;
int BACK = 0;
int nowLineNum = 1;
int renderingLineNum = 1;
int colorSet = 1;

bool find(const string s, const vector<Token> v) {
    for (auto autoV : v)
        if (s == autoV.word)
            return true;
    return false;
}

void backChange(int back) {
    // using 0x79
    init_pair(NOMAL, 0x69, back);        // 黒地に白文字
    init_pair(TYPE, 0x33, back);         // 黒地に水色文字
    init_pair(MACRO, 0x38, back);        // 黒地に青文字
    init_pair(NUMBER, 0xCA, back);       // 黒地にオレンジ文字
    init_pair(PARENTHESES, 0x4B, back);  // 黒地に黄色文字
    init_pair(BRACKETS, 0xE2, back);     // 黒地に黄色文字
    init_pair(VARIABLE, 0x74, back);     // 黒地に黄色文字
    init_pair(RESERVED, 0xD5, back);     // 黒地にオレンジ文字
    init_pair(OP, 0x9D, back);     // 黒地にオレンジ文字
    init_pair(COMMANDLINE, 0xE1, 0xEC);  
    init_pair(STATUS, 0xE1, 0x3C); 
    init_pair(SUBWIN, 0x69, 0x5A);                                    
    init_pair(SP_RESERVED, 0x1F, back);
}

bool split_token(string::iterator data, const char *word, int index) {
     for (int i = 0; i < index; i++)
        if (data[i] != word[i])
            return false;
    return true;
}

int lineTop(const int inOffset) {
    int offset = inOffset - 1;
    while (offset >= 0 && gBuf[offset] != '\n') offset--;
    return offset <= 0 ? 0 : ++offset;
}

int nextLineTop(const int inOffset) {
    int offset = inOffset;
    while (offset < gBuf.size() && gBuf[offset++] != '\n') { /* empty */ }
    return offset < gBuf.size() ? offset : gBuf.size() - 1;
}

int adjust(const int inOffset, const int inCol) {
    int offset = inOffset;
    for (int i = 0; offset < gBuf.size() && gBuf[offset] != '\n' && i < inCol;offset++)
        i += gBuf[offset] == '\t' ? 4 - (i & 3) : 1;
    return offset;
}

auto putColor(std::__wrap_iter<char*>  p) {
    return p;
}

void tokenPaint(int  *nowToken, int *tokenCounter, int len, int attribute) {
    if (*tokenCounter == 0) *tokenCounter = len;
    (*tokenCounter)--;
    *nowToken = attribute;
    attrset(COLOR_PAIR(attribute));
}

void display() {
    int howChangeStart = 0, howManyEnter = 0;

    // 上にスクロール
    if (gIndex <= gPageStart)
        gPageStart = lineTop(gIndex);

    // 下の行のデータを取得
    if (gPageEnd <= gIndex) {
        gPageStart = nextLineTop(gIndex); // 次のラインのインデックスを取得 
        int n = LINES-1; // ファイル容量 - コマンド表示欄分マイナス 

        for (int i = 0; i < n; i++)
            gPageStart = lineTop(gPageStart - 1);
    }

    move(0, 0);
    int i = 0, j = 0;
    gPageEnd = gPageStart;

    int tokenCounter = 0, nowToken = 0;

    for (auto p = gBuf.begin() + gPageEnd; /*empty */; gPageEnd++, p++) {
        if (gIndex == gPageEnd) {
            // update cursor pos.
            gRow = i;
            gCol = j;
        } 
        if (LINES-1 <= i || gBuf.size() <= gPageEnd)
            break;
        if (*p != '\r') {
            // if colour options are set
            switch (*p) {
                case '<':
                case '>':
                case '+':
                case '-':
                case '*':
                case '/':
                case '=':
                case '&':
                case '$':
                case '%':
                    attrset(COLOR_PAIR(OP));
                    break;

                case '(':
                case ')':
                    attrset(COLOR_PAIR(PARENTHESES));
                    break;

                case '{':
                case '}':
                    attrset(COLOR_PAIR(BRACKETS));
                    break;

                default: {
                    if (isdigit(*p)) attrset(COLOR_PAIR(NUMBER));

                    /* ===--- TYPE ---=== */
                    else if (split_token(p, "int ", 4) ||
                             (nowToken == TYPE && tokenCounter > 0))
                        tokenPaint(&nowToken, &tokenCounter, 4, TYPE);

                    else if (split_token(p, "char ", 5) ||
                             (nowToken == TYPE && tokenCounter > 0))
                        tokenPaint(&nowToken, &tokenCounter, 5, TYPE);

                    /* ===--- MACRO ---=== */
                    else if (split_token(p, "#define ", 8) ||
                             (nowToken == MACRO && tokenCounter > 0))
                        tokenPaint(&nowToken, &tokenCounter, 8, MACRO);

                    else if (split_token(p, "#include ", 9) ||
                             (nowToken == MACRO && tokenCounter > 0))
                        tokenPaint(&nowToken, &tokenCounter, 9, MACRO);

                    /* ===--- RESERVED ---=== */
                    else if (split_token(p, "if ", 3) ||
                             (nowToken == RESERVED && tokenCounter > 0))
                        tokenPaint(&nowToken, &tokenCounter, 3, RESERVED);

                    else if (split_token(p, "for ", 4) ||
                             (nowToken == RESERVED && tokenCounter > 0))
                        tokenPaint(&nowToken, &tokenCounter, 4, RESERVED);

                    else if (split_token(p, "while ", 6) ||
                             (nowToken == RESERVED && tokenCounter > 0))
                        tokenPaint(&nowToken, &tokenCounter, 6, RESERVED);

                    else if (split_token(p, "return ", 7) ||
                             (nowToken == RESERVED && tokenCounter > 0))
                        tokenPaint(&nowToken, &tokenCounter, 7, RESERVED);

                    /* ===--- SP_RESERVED ---=== */
                    else if ((split_token(p, "printf", 6) ||
                              split_token(p, "using ", 6) ||
                              (split_token(p, "class ", 6))) ||
                             (nowToken == SP_RESERVED && tokenCounter > 0))
                        tokenPaint(&nowToken, &tokenCounter, 6, SP_RESERVED);

                    else if (split_token(p, "def ", 4) ||
                             (nowToken == SP_RESERVED && tokenCounter > 0))
                        tokenPaint(&nowToken, &tokenCounter, 4, SP_RESERVED);

                    /* ===--- ELSE ---=== */
                    else {
                        tokenCounter = 0;
                        attrset(COLOR_PAIR(NOMAL));
                    }
                    break;
                }
            }
            addch(*p);
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
    string cursorRow = " " + fileName + " " + to_string(nowLineNum) + "*" + to_string(gCol+1) + " ";

    attrset(COLOR_PAIR(STATUS));
    mvaddstr(i, 0, commandLineWord.c_str());
    attrset(COLOR_PAIR(COMMANDLINE));
    for (auto j=commandLineWord.size(); j < COLS - cursorRow.size();)
        mvaddstr(i, j++, " ");
    attrset(COLOR_PAIR(STATUS));
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

vector<Token> initPredictiveTransform() {
    vector<Token> vec {
        Token {"#define", MACRO},
        Token {"#include", MACRO},
        Token {"char", TYPE},
        Token {"const", SP_RESERVED},
        Token {"else", RESERVED},
        Token {"elif", RESERVED},
        Token {"for", RESERVED},
        Token {"if", RESERVED},
        Token {"int", TYPE},
        Token {"map", TYPE},
        Token {"return", RESERVED},
        Token {"void", TYPE},
        Token {"vector", TYPE},
        Token {"while", RESERVED},
    };
    return vec;
}

// 

vector<Token> predictiveWin(const string word, const vector<Token> vec, const int index) {
    int i = 0;
    int ignoreBuf = 0;
    int type;
    bool ok = false;
    string data;
    vector<Token> newVec;

    savetty();

    for (int j = 0; j < vec.size(); data = vec[j].word, type = vec[j].type, j++) {
        attrset(COLOR_PAIR(COMMANDLINE));
        if (data[0] == word[0])
            ok = true;
        if (data[0] != word[0])
            ok = false;
        if (data[word.size() - 1] == word[word.size() - 1] && ok == true) {
            if (index+ignoreBuf == j)
                attrset(COLOR_PAIR(STATUS));

            mvaddstr(gRow + ++i, gCol, data.c_str());
            newVec.push_back(Token{data, type});
        }
        else ignoreBuf++;
    }
    resetty();

    return newVec;
}

// Command
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

void gotoUp() {
    gIndex = adjust(gPageStart, 0);
}

void gotoDown() {
    gIndex = adjust(gPageEnd-1, 0);
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

// no move
void save() {
    ofstream ofs(gFileName, ios::binary);
    ostream_iterator<char> output_iterator(ofs);
    copy(gBuf.begin(), gBuf.end(), output_iterator);
    commandLineWord = "===--- saved ---===";
    redraw();
}

void renderingNowLine() {
    string number = " " + to_string(nowLineNum) + " : " + to_string(gCol) + " ";
    
    savetty();

    attrset(COLOR_PAIR(SUBWIN));
    mvaddstr(gRow+1, gCol, number.c_str());
    move(gRow, gCol);
    refresh();
    getch();

    resetty();
}


int index(string word, const vector<char> vec) {
    int v=0;
    bool passd = false;

    return 0;
}

void wordJump() {

    int i = index("main", gBuf);
    gIndex = (i > 0) ? i : gIndex;
    redraw();
}

// Command input
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
    if (ch == '/')
        wordJump();

    commandLineWord = before_commandLineWord;
    redraw();
}

void newLine() {
    lineEnd();
    gBuf.insert(gBuf.begin() + gIndex, '\n');
    gIndex++;
    redraw();
    insertMode();
}

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

template <typename T>
void slice(std::vector<T> &v, int m, int n) {
    auto first = v.cbegin() + m;
    auto last = v.cbegin() + n + 1;
 
    v.erase(first, last);
}

// insertMode 
static void insertMode() {
    commandLineWord = "! insert !";
    redraw();
    int viewIndex = -1;
    int tmpBuf;
    bool tabStart = false;
    string nowInputWord;
    vector<Token> newPredictive;

    for (int ch;;display(), savetty()) {
        redraw();
        newPredictive.clear();
        newPredictive = predictiveWin(nowInputWord, predictive, viewIndex);

        move(gRow, gCol);

        if ((ch = getch()) == kESC) {
            viewIndex = -1;
            break;
        }

        else if (gIndex > 0 && (ch == kBS || ch == kDEL)) {
            gBuf.erase(gBuf.begin() + (--gIndex));
            if (nowInputWord.size() > 0)nowInputWord.resize(nowInputWord.size()-1);
            nowLineNum--;
            viewIndex = -1;
        }

        else if (ch == kCtrlN) { 
            if (viewIndex <= -1) gBuf.erase(gBuf.begin() + (--gIndex));

            for (auto ch : newPredictive[viewIndex].word)
                gBuf.erase(gBuf.begin() + (--gIndex));

            (viewIndex < newPredictive.size()-1) ? viewIndex++ : viewIndex = 0;

            for (auto ch : newPredictive[viewIndex].word)
                gBuf.insert(gBuf.begin() + gIndex++, ch == '\r' ? '\n' : ch);

            newPredictive.clear();
        }

        else if (ch == kCtrlP) {

            for (auto ch : newPredictive[viewIndex].word)
                gBuf.erase(gBuf.begin() + (--gIndex));

            (viewIndex > -1) ? viewIndex-- : viewIndex = newPredictive.size()-1;

            for (auto ch : newPredictive[viewIndex].word)
                gBuf.insert(gBuf.begin() + gIndex++, ch == '\r' ? '\n' : ch);

            newPredictive.clear();
        } 

        else if ((ch>='a'&& ch<='z') || (ch>='A' && ch<='Z') || (ch>='0'&& ch<='9')) {
            gBuf.insert(gBuf.begin() + gIndex++, ch == '\r' ? '\n' : ch);
            nowInputWord.push_back(ch);
            /* 
            //DEBUG:
            clear();
            printw("%s\n", nowInputWord.c_str());
            refresh();
            exit(0);
            */

        }

        else {
            gBuf.insert(gBuf.begin() + gIndex++, ch);

            if (ch == '\n') {
                (gBuf[lineTop(gIndex)] == '\t')
                   ? gBuf.insert(gBuf.begin() + gIndex++, '\t')
                   : std::vector<char>::iterator();
                nowLineNum++;
            }


            viewIndex = -1;
            predictive.push_back(Token{nowInputWord, NOMAL});
            predictive.push_back(Token{"", 0});
            nowInputWord.clear();

        }
        usleep(5000);
    }
    resetty();
    commandLineWord = " NOMAL ";
}

// visualMode
static void visualMode() {
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
                for (;gBuf[gIndex] != '\n';) {gIndex++;};
                gIndex++;

                int i = 0;
                for (;tmp_gIndex < gIndex; ++moveDiff, i++)
                    yankBuf.push_back(gBuf[gIndex--]);

                gIndex += i;

                redraw();
                break;
            }

            case 'k': {
                int tmp_gIndex = gIndex;
                int i = 0;

                for (;gBuf[gIndex-1] != '\n';) {gIndex--;};
                gIndex--;

                for (;gIndex < tmp_gIndex; --moveDiff, i++)
                    yankBuf.push_back(gBuf[gIndex++]);

                gIndex -= i;

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
                if (++moveDiff >= 0) {
                    slice(gBuf, base_gIndex, gIndex);
                    gIndex = base_gIndex;
                } else 
                    slice(gBuf, gIndex, base_gIndex);
                redraw();
                return;
            }

            case kESC:
                break;
        }
    }
}

void paste() {
    reverse(yankBuf.begin(), yankBuf.end());
    for (auto ch : yankBuf)
        gBuf.insert(gBuf.begin() + gIndex++, ch == '\r' ? '\n' : ch);
}

void addInsert() {
    gIndex++;
    redraw();
    insertMode();
}

void undo() {
    gBuf.clear();
    copy(gUndoBuf.begin(), gUndoBuf.end(), back_inserter(gBuf) );
    gBuf = gUndoBuf;
    redraw();
}

unordered_map<char, void (*)()> gAction = {
    {'h', left},        {'l', right},       {'k', up},
    {'j', down},        {'b', wordLeft},    {'w', wordRight},
    {kCtrlD, pageDown}, {kCtrlU, pageUp},   {'0', lineBegin},
    {'$', lineEnd},     {'T', top},         {'G', bottom},
    {'i', insertMode},  {'x', del},         {kCtrlQ, quit},
    {kCtrlR, redraw},   {kCtrlS, save},     {'o', newLine},
    {'H', gotoUp},      {'L', gotoDown},
    {'f', commandMode}, {':', commandMode}, {' ', commandMode},
    {'d', del},         {'c', lineBegin},   {'w', oneWordMove},
    {'b', oneWordBack}, {'/', gotoLine},    {'v', visualMode},
    {'p', paste},       {'a', addInsert},   {'u', undo},

};

int main(int argc, char **argv) {
    if (argc < 2)
        return 2;

    initscr();

    // 色設
    start_color();
    BACK = 0xEA;
    assume_default_colors(0, BACK);
    backChange(BACK);
    attrset(COLOR_PAIR(1));
    
    set_tabsize(4);
    raw();
    noecho();
    idlok(stdscr, true);  // init screen.
    getmaxyx(stdscr, h, w);
    gFileName = argv[1];
    ifstream ifs(gFileName, ios::binary);
    gBuf.assign(istreambuf_iterator<char>(ifs), istreambuf_iterator<char>());
    gUndoBuf = gBuf;
    commandLineWord = "> ";
    string nowToken;
    for (auto data : gBuf) {
        if ((data>='a'&& data<='z') || (data>='A' && data<='Z') || (data>='0'&& data<='9'))
            nowToken.push_back(data);

        else if (!find(nowToken, predictive)){
            predictive.push_back(Token{nowToken, NOMAL});
            predictive.push_back(Token{"", 0});
            nowToken.clear();
        }
    }
    while (!gDone) {
        display();
        char ch = getch();
        if (gAction.count(ch) > 0)
            gAction[ch]();
    }
    endwin();
    return 0;
}
