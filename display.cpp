#include "schwarz.hpp"
#include "move.hpp"
#include "util.hpp"

void display() {

    int howChangeStart = 0, howManyEnter = 0;

    // 上に移動
    if (gIndex <= gPageStart) {
        (gIndex > 0) ? LineStart-- : 0;
        gPageStart = lineTop(gIndex); // 前のラインのインデックスを取得
    }

    // 下に移動
    if (gPageEnd <= gIndex) {
        // 潜在的なバグ
        (LineStart < gLines-(h-1)) ? LineStart++ : 0;
        gPageStart = nextLineTop(gIndex); // 次のラインのインデックスを取得 
        int n = LINES-1; // ファイル容量 - コマンド表示欄分マイナス 

        for (int i = 0; i < n; i++)
            gPageStart = lineTop(gPageStart - 1);
    }

    move(0, 0);
    int i = 0;
    int j = 0;
    int c = (gLines > h) ? 0 : 1;
    int tokenCounter = 0;
    int nowToken = 0;
    int AllLineLength = to_string(gLines).size()+1;
    string lineNumberString = to_string(LineStart + c++) + " ";

    gPageEnd = gPageStart;

    for (;AllLineLength > lineNumberString.size();)
        lineNumberString += " ";

    nowLineBuf = lineNumberString.size();
    if (finderSwitch)
        nowLineBuf += finderData[0].length();

    attrset(COLOR_PAIR(LINE));
    printw("%s", (((finderData.size() > i && finderSwitch) ?finderData[i] : "") + lineNumberString).c_str());

    for (auto p = gBuf.begin() + gPageEnd;; gPageEnd++, p++) {
        if (gIndex == gPageEnd) {
            // update cursor pos.
            gRow = i;
            gCol = j;
        } 

        if (LINES-1 <= i || gBuf.size() <= gPageEnd)
            break;

        if (*p != '\r') {
            // if the colour options was set
            switch (*p) {
                case '<':
                case '>':
                case '+':
                case '-':
                case '*':
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

                    if (!isdigit(*p)) {
                        vector<Token> vec = initPredictiveTransform();
                        for (auto v : vec) {
                            if (isFunction(p)) {
                                tokenPaint(&nowToken, &tokenCounter, 1, FUNCTION);
                                break;
                            }

                            if (split_token(p, v.word.c_str(), v.word.size()) ||
                                (nowToken == v.type && tokenCounter > 0)) {
                                tokenPaint(&nowToken, &tokenCounter, v.word.size(), v.type);
                                break;
                            }
                            attrset(COLOR_PAIR(NOMAL));
                        }
                    }
                    else attrset(COLOR_PAIR(NUMBER));
                    break;
                }
            }

            (*p == '\t') ? printw("    ") : addch(*p);

            j += *p == '\t' ? 4 - (j & 3) : 1;
        }
        if (*p == '\n' || COLS <= j) {

            if (!(LineStart + c < gLines+1))  {
                break;
            }

            lineNumberString = to_string(LineStart + c++) + " "; 

            for (; AllLineLength > lineNumberString.size() ;lineNumberString += " ");
            attrset(COLOR_PAIR(LINE));

            i++;
            if (finderSwitch) {
                if (finderData.size() < i)
                    for (int k=0;k < nowLineBuf-1; k++)
                        printw(" ");
                else {
                    printw("%s", finderData[i].c_str());

                    for (int k = finderData[i].size(); k < nowLineBuf-1; k++)
                        printw(" ");
                }
            }
            printw("%s", (lineNumberString).c_str());
            j = 0;
        }
    }

    attrset(COLOR_PAIR(NOMAL));

    for (;++i < h-1;)
        mvaddstr(i, 0, "~");

    if (gRow < LINES-1)
        gRow += howChangeStart;

    string fileName = gFileName;
    string cursorRow = " " + fileName + " " + to_string(nowLineNum) + "*" + to_string(gCol+1) + " ";

    attrset(COLOR_PAIR(nowMode));
    mvaddstr(i, 0, commandLineWord.c_str());
    attrset(COLOR_PAIR(COMMANDLINE));

    for (auto j=commandLineWord.size(); j < COLS - cursorRow.size();)
        mvaddstr(i, j++, " ");

    attrset(COLOR_PAIR(STATUS));
    mvaddstr(i, COLS - cursorRow.size(), cursorRow.c_str());
    clrtobot();

    // カーソルの表示
    move(gRow, gCol+nowLineBuf);
    refresh(); 
}

