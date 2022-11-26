// main.cpp   mushi's editor 2022.
// compile: g++ -std=c++1z main.cpp -lcurses
// Github; Dangornushi/schwarz
//DEBUG:
/*
clear();
printw("%s\n", nowInputWord.c_str());
refresh();
exit(0);
*/

#include "schwarz.hpp"
#include "move.hpp"
#include "util.hpp"
#include "finder.hpp"
#include "visualMode.hpp"

const char *gFileName;

vector<Token> predictive = initPredictiveTransform();
vector<char> gBuf, gUndoBuf;
vector<string> finderData;
string commandLineWord;
string yankBuf;

bool gDone = false;
int gIndex;
int gPageStart;
int gPageEnd;
int gLines;
int gCol, gRow;
int gUndoIndex = 0;

int nowLineNum;
int LineStart;
int LineEnd;
int w, h;

int moveDiff;
int nowMode;
int BACK;
int renderingLineNum;
int colorSet;
int nowLineBuf;
int nowWindow;
int windows;
int visualStart;
int visualEnd;
bool classical;
bool finderSwitch;

void globalInit() {
    nowMode = NOMAL_MODE;
    gIndex = 0 /* offset of cursor pos */;
    gPageStart = 0;
    gPageEnd = 0;
    gLines = 0;
    gRow = 0;
    gCol = 0;

    nowLineNum = 1;
    LineStart = 0;
    LineEnd = 0;
    BACK = 0;
    renderingLineNum = 1;
    colorSet = 1;
    
    visualStart = 0;
    visualEnd = 0;
}

// TAG: Tokens
vector<Token> initPredictiveTransform() {
    vector<Token> vec {
        // all
        Token {"else ", RESERVED},
        Token {"if ", RESERVED},
        Token {"int ", TYPE},
        Token {"return ", RESERVED},
        Token {"for ", RESERVED},
        Token {"void ", TYPE},
        Token {"while ", RESERVED},
        Token {"class ", RESERVED},

        // C/C++
        Token {"break", RESERVED},
        Token {"char ", TYPE},
        Token {"const ", SP_RESERVED},
        Token {"case ", RESERVED},
        Token {"typedef ", SP_RESERVED},
        Token {"struct ", RESERVED},
        Token {"map", TYPE},
        Token {"namespace ", RESERVED},
        Token {"vector", TYPE},
        Token {"bool ", TYPE},
        Token {"string ", TYPE},
        Token {"#define ", MACRO},
        Token {"#include ", MACRO},
        Token {"printf", RESERVED},
        Token {"public", SP_RESERVED},
        Token {"private", SP_RESERVED},
        Token {"uint32", TYPE},
        Token {"uint8", TYPE},
        Token {"using ", SP_RESERVED},
        Token {"/*", CONSECUTIVECOMMENT},
        Token {"*/", CONSECUTIVECOMMENT},
        Token {"//", COMMENT},

        // Python
        Token {"elif ", RESERVED},
        Token {"lamda ", RESERVED},
        Token {"def ", RESERVED},
        Token {"__init__ ", RESERVED},
        Token {"__main__ ", RESERVED},
        Token {"self", TYPE},
    };
    return vec;
}

// TAG: predict
// 予測変換のサブウィンドウ
vector<Token> predictiveWin(const string word, const vector<Token> vec, const int index) {
    int i = 0;
    int ignoreBuf = 0;
    int type;
    bool ok = false;
    string data;
    vector<Token> newVec;

    if (vec.size() < index+1)
        return vec;

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

            mvaddstr(gRow + ++i, gCol+nowLineBuf, data.c_str());
            newVec.push_back(Token{data, type});
        }
        else ignoreBuf++;
    }
    resetty();

    return newVec;
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
    move(gRow, gCol+nowLineBuf);
    refresh();
    getch();

    resetty();
}

void wordJump() {
    
    string jumpWordBuf;
    int jumpWordIndex = 0;

    for (int ch;;display(), savetty()) {
        if ((ch = getch()) == kESC)
            break;

        else if (jumpWordIndex > 0 && (ch == kBS || ch == kDEL)) {
            jumpWordBuf.erase(jumpWordBuf.begin() + (--jumpWordIndex));
            nowLineNum--;
        }

        else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
                 (ch >= '0' && ch <= '9') || (ch == '_'))
            jumpWordBuf.insert(jumpWordBuf.begin() + jumpWordIndex++, ch == '\r' ? '\n' : ch);

        else {
            jumpWordBuf.insert(jumpWordBuf.begin() + jumpWordIndex++, ch);

            if (ch == '\n')
                break;

        }
        commandLineWord = jumpWordBuf;
    }
    index(jumpWordBuf, gBuf);
    redraw();
}

void renderingFinder() {
    string gFileNameBuf;
    int winW = 0; 
    int winH = (h-1);

    savetty();
    
    attrset(COLOR_PAIR(COMMANDLINE));
    for (int ch;;) {
        if ((ch = getch()) == kESC)
            break;

        else if ((ch == kBS) || (ch == kDEL))
            gFileNameBuf.erase(gFileNameBuf.end()-1);

        else if (ch == '\n')
                break;
        else 
            gFileNameBuf.insert(gFileNameBuf.end(), ch);

        move(winH, 0);
        clrtoeol();
        mvaddstr(winH, winW, gFileNameBuf.c_str());
    }

    gFileName = gFileNameBuf.c_str();

    clear();
    refresh();
    resetty();
    globalInit();
    run();
}

// Command input
void commandMode() {
    nowMode = COMMAND_MODE;
    string before_commandLineWord = commandLineWord;
    commandLineWord = "> ";
    redraw();

    for (int ch = getch();;) {
        commandLineWord = ch;
        redraw();

        if (kESC == ch)
            break;

        if ('w' == ch) {
            save();
            break;
        }

        if ('q' == ch) {
            quit();
            break;
        }

        if ('o' == ch) {
            renderingFinder();
            break;
        }


        if ('l' == ch) {
            renderingNowLine();
            break;
        }
        break;
    }

    //commandLineWord = before_commandLineWord;
    nowMode = NOMAL_MODE;
    redraw();
}

void newLine() {
    lineEnd();
    gBuf.insert(gBuf.begin() + gIndex++, '\n');
    (gBuf[lineTop(gIndex - 1)] == '\t')
        ? gBuf.insert(gBuf.begin() + gIndex++, '\t')
        : std::vector<char>::iterator();
    nowLineNum++;
    gLines++;

    gCol=0;
    gRow++;
    clear();
    redraw();
    insertMode();
}

void lineDel() {
    /*
    for (; gBuf[gIndex] != '\n'; gIndex++) {
        ++moveDiff;
        yankBuf.push_back(gBuf[gIndex]);
    }
    for (int i = 0; i < gCol; i++)
        yankBuf.push_back(gBuf[gIndex++]);
    */
}

// insertMode 
void insertMode() {
    nowMode = INSERT_MODE;
    commandLineWord = "! insert !";
    redraw();
    int viewIndex = -1;
    int tmpBuf, tmpIndex=gIndex;
    bool tabStart = false;
    string nowInputWord;
    vector<Token> newPredictive;

    gUndoBuf = gBuf;
    gUndoIndex = gIndex;

    for (int ch;;redraw(), savetty()) {
        if (!classical) {
            newPredictive.clear();
            newPredictive = predictiveWin(nowInputWord, predictive, viewIndex);
        }

        move(gRow, gCol+nowLineBuf+1);

        if ((ch = getch()) == kESC) {
            viewIndex = -1;
            break;
        }

        else if (gIndex > 0 && (ch == kBS || ch == kDEL)) {
            (gBuf[--gIndex] == '\n')
                ? gLines--
                : 0;

            gBuf.erase(gBuf.begin() + (gIndex));
            if (nowInputWord.size() > 0)nowInputWord.resize(nowInputWord.size()-1);
            nowLineNum--;
            viewIndex = -1;
        }

        else if (!classical && ch == kCtrlN) {

            if (viewIndex == -1)
                // CtrlNの一番上
                for (int i = gIndex; i > tmpIndex; i--, gIndex--)
                    gBuf.erase(gBuf.begin() + gIndex - 1);

            else
                // 一番上以外
                for (int i = 0; i < newPredictive[viewIndex].word.size(); i++)
                    gBuf.erase(gBuf.begin() + (gIndex--) - 1);

            (viewIndex < newPredictive.size()) ? viewIndex++ : viewIndex = 0;

            for (int i = 0; i < newPredictive[viewIndex].word.size(); i++)
                gBuf.insert(gBuf.begin() + gIndex++,
                            newPredictive[viewIndex].word[i] == '\r'
                                ? '\n'
                                : newPredictive[viewIndex].word[i]);

            newPredictive.clear();
        }

        else if (!classical && ch == kCtrlP) {
            if (newPredictive.size() < viewIndex)
                continue;

            for (auto ch : newPredictive[viewIndex].word)
                gBuf.erase(gBuf.begin() + (--gIndex));

            (viewIndex > -1) ? viewIndex-- : viewIndex = newPredictive.size()-1;

            for (auto ch : newPredictive[viewIndex].word)
                gBuf.insert(gBuf.begin() + gIndex++, ch == '\r' ? '\n' : ch);

            newPredictive.clear();
        }

        else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
                 (ch >= '0' && ch <= '9') || (ch == '_')) {
            gBuf.insert(gBuf.begin() + gIndex++, ch == '\r' ? '\n' : ch);
            (!classical) ? nowInputWord.push_back(ch) : void();
        }

        else {
            gBuf.insert(gBuf.begin() + gIndex++, ch);

            if (ch == '\n') {
                int spaceCounter = 0;

                for (int i = lineTop(gIndex-1) ;gBuf[i] == '\t' || gBuf[i] == ' '; i++, spaceCounter++);

                for (int i =0;i < spaceCounter; i++)
                    gBuf.insert(gBuf.begin() + gIndex++, '\t');

                nowLineNum++;
                gLines++;
            }
            tmpIndex = gIndex;

            viewIndex = -1;

            if (!classical) {
                predictive.push_back(Token{nowInputWord, NOMAL});
                nowInputWord.clear();
            }
        }
    }
    resetty();
    nowMode = NOMAL_MODE;
    commandLineWord = " NOMAL ";
}

void paste() {
    (moveDiff < 0) ? reverse(yankBuf.begin(), yankBuf.end()) : void();
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
    gIndex = gUndoIndex;
    redraw();
}

void windowChange() {
    (windows > nowWindow && windows > 1) ? nowWindow++ : nowWindow = 0;
}

unordered_map<char, void (*)()> gAction = {
    {'h', left},        {'l', right},       {'k', up},
    {'j', down},        {'b', wordLeft},    {'w', wordRight},
    {kCtrlD, pageDown}, {kCtrlU, pageUp},   {'0', lineBegin},
    {'$', lineEnd},     {'T', top},         {'G', bottom},
    {'i', insertMode},  {'x', del},         {kCtrlQ, quit},
    {kCtrlR, redraw},   {kCtrlS, save},     {'o', newLine},
    //{kCtrlW, finderCursor},
    {kCtrlF, finder},
    {'H', gotoUp},      {'L', gotoDown},
    {'f', commandMode}, {':', commandMode}, {' ', commandMode},
    {'d', del},         {'c', lineBegin},   {'w', oneWordMove},
    {'b', oneWordBack}, {'/', wordJump},    {'v', visualMode},
    {'p', paste},       {'a', addInsert},   {'u', undo},

};

void run() {
    ifstream ifs(gFileName, ios::binary);
    gBuf.assign(istreambuf_iterator<char>(ifs), istreambuf_iterator<char>());
    gUndoBuf = gBuf;    
    gUndoIndex = gIndex;

    nowMode = NOMAL_MODE;
    nowWindow = 0;
    windows = 1;
    commandLineWord = " NOMAL ";
    finderData = {""};
    finderSwitch = false;

    // split token
    if (!classical) {
        // 色設
        start_color();

        if (can_change_color() == true && has_colors() == true )
            backChange();

        assume_default_colors(BACK, BACK);

        string nowToken;

        for (auto data : gBuf) {
            if (isChar(data))
                nowToken.push_back(data);

            if (data == '\n')
                gLines++;

            else if (!find(nowToken, predictive)) {
                predictive.push_back(Token{nowToken, NOMAL});
                predictive.push_back(Token{"", 0});
                nowToken.clear();
            }
        }
    }

    // start
    while (!gDone) {
        display();
        char ch = getch();
        if (gAction.count(ch) > 0)
            gAction[ch]();

        if (nowWindow == 1)
            finder();
    }
    endwin();
}

void init() {
    // init
    initscr();
    setlocale(LC_ALL, "");
    set_tabsize(4);
    raw();
    noecho();
    idlok(stdscr, true);  // init screen.
    getmaxyx(stdscr, h, w);
    
	classical = false;

    run();
}

int main(int argc, char **argv) {
	if (argc < 2)
		return 2;

    // file
    gFileName = argv[1];
    init();

    return 0;
}

