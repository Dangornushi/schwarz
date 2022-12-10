#include "schwarz.hpp"
#include "move.hpp"
#include "util.hpp"
#include "finder.hpp"

void display() {

    int c = 1;

    if (gIndex <= gPageStart) {
        // 上に移動
        (gIndex > 0) ? LineStart-- : 0;
        gPageStart = lineTop(gIndex);
    }

    else if (gPageEnd < gIndex) {
        // 下に移動
        
        (LineStart < (h-1)) ? LineStart++ : 0;
        gPageStart = nextLineTop(gIndex);

        // ファイル容量 - コマンド表示欄分マイナス 
        int n = gPageStart == gBuf.size() - 1 ? LINES - 1 : LINES;
        for (int i = 0; i < n; i++) {
            gPageStart = lineTop(gPageStart - 1);
        }
    }

    move(0, 0);

    int x = 0;
    int y = 0;
    //int j = 0;
    int tokenCounter = 0;
    int nowToken = 0;
    int AllLineLength = to_string(gLines).size()+1;
    string lineNumberString;
    bool nowComment = false;
    bool nowConsecutiveComment = false;

    gPageEnd = gPageStart;

    if (finderData.size() > 0 && finderSwitch) {
        attrset(COLOR_PAIR(NOMAL)); printw(" >%s", finderData[0].c_str()); }

    nowLineBuf = 0;

    drawLinenumAndFinder(&lineNumberString, &c, AllLineLength);

    //if (finderSwitch)
    //    nowLineBuf += finderData[0].length();

    for (auto p = gBuf.begin() + gPageEnd;; gPageEnd++, p++) {

        if (gIndex == gPageEnd) {
            // update cursor pos.
            gRow = y;
            gCol = x;
        } 

        if (y == h - 1 || gBuf.size() <= gPageEnd)
            break;

        if (nowMode == VISUAL_M &&
            ((gPageEnd >= visualStart && gPageEnd < visualEnd) ||
             (gPageEnd < visualStart && gPageEnd >= visualEnd))) {

            attrset(COLOR_PAIR(COMMANDLINE));

            (*p == '\t') ? printw("    ") : addch(*p);
            x += *p == '\t' ? 4 - (x & 3) : 1;
            if (LineStart + c > gLines + 1) {
                y++;
                break;
            }

            if (*p == '\n' || COLS <= x) {
                drawInDir(finderSwitch, lineNumberString, ++y);
                drawLinenumAndFinder(&lineNumberString, &c, AllLineLength);

                x = 0;
            }
            continue;
        }

        
        if ((LineStart + c) > gLines) {
            y++;
            nowComment = false;
        }

        else;

        if (*p == '\n' || COLS <= x) {
            printw("\n");
            drawInDir(finderSwitch, lineNumberString, ++y);
            drawLinenumAndFinder(&lineNumberString, &c, AllLineLength);
            x = 0;
            nowComment = false;
            attrset(COLOR_PAIR(NOMAL));
        }
        //if (*p != '\r') {
        else {
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
                    if (nowComment || nowConsecutiveComment) {
                        if (*p == '/') {
                            nowConsecutiveComment = false;
                            nowComment = false;
                        }
                        attrset(COLOR_PAIR(COMMENT));
                        break;
                    }

                    if (!isdigit(*p)) {
                        vector<Token> vec = initPredictiveTransform();
                        for (auto v : vec) {
                            if (isFunction(p)) {
                                tokenPaint(&nowToken, &tokenCounter, 1, FUNCTION);
                                break;
                            }

                            if (split_token(p, v.word.c_str(), v.word.size()) ||
                                (nowToken == v.type && tokenCounter > 0)) {
                                if (v.type == COMMENT)
                                    nowComment = true;
                                if (v.type == CONSECUTIVECOMMENT)
                                    nowConsecutiveComment = true;
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

            x += *p == '\t' ? 4 - (x & 3) : 1;
        }

    }

    move(y, 0);
    attrset(COLOR_PAIR(NOMAL));

    while (y <= h-1) { 
        drawInDir(finderSwitch, lineNumberString, y++);
        lineNumberString = "~\n";
        drawTildeAndFinder(&lineNumberString,  AllLineLength);
    }
    string fileName = gFileName;
    string cursorRow = " " + fileName + " " + to_string(nowLineNum) + "*" + to_string(gCol+1) + " ";
    int consoleRow = h - 1;

    attrset(COLOR_PAIR(nowMode));
    mvaddstr(consoleRow, 0, commandLineWord.c_str());
    attrset(COLOR_PAIR(COMMANDLINE));

    for (auto j=commandLineWord.size(); j < COLS - cursorRow.size();)
        mvaddstr(consoleRow, j++, " ");

    attrset(COLOR_PAIR(STATUS));
    mvaddstr(consoleRow, COLS - cursorRow.size(), cursorRow.c_str());
    clrtobot();

    // カーソルの表示
    move(gRow, gCol+nowLineBuf+1);
    refresh(); 
}

