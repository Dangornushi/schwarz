#include "schwarz.hpp"
#include "move.hpp"
#include "util.hpp"
#include "finder.hpp"

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
    string lineNumberString;
    bool nowComment = false;
    bool nowConsecutiveComment = false;

    gPageEnd = gPageStart;

    if (finderData.size() > i && finderSwitch) {
        attrset(COLOR_PAIR(NOMAL)); printw("%s", finderData[i].c_str()); }

    drawLinenumAndFinder(&lineNumberString, &c, AllLineLength);

    nowLineBuf = lineNumberString.size();

    if (finderSwitch)
        nowLineBuf += finderData[0].length()+1;

    for (auto p = gBuf.begin() + gPageEnd;; gPageEnd++, p++) {
        if (gIndex == gPageEnd) {
            // update cursor pos.
            gRow = i;
            gCol = j;
        } 

        if (LINES-1 <= i || gBuf.size() <= gPageEnd)
            break;

        if (nowMode == VISUAL_M &&
            ((gPageEnd > visualStart && gPageEnd < visualEnd) ||
             (gPageEnd < visualStart && gPageEnd > visualEnd))) {

            attrset(COLOR_PAIR(COMMANDLINE));
            (*p == '\t') ? printw("    ") : addch(*p);
            j += *p == '\t' ? 4 - (j & 3) : 1;
            if (LineStart + c >= gLines + 1) {
                i++;
                break;
            }

            if (*p == '\n' || COLS <= j) {
                drawInDir(finderSwitch, lineNumberString, ++i);
                drawLinenumAndFinder(&lineNumberString, &c, AllLineLength);

                j = 0;
            }
            continue;
        }

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
                                if (v.type == CONSECUTIVECOMMENT) {
                                    nowConsecutiveComment = true;
                                }
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

        if (LineStart + c >= gLines + 1) {
            i++;
            nowComment = false;
            break;
        }

        if (*p == '\n' || COLS <= j) {
            drawInDir(finderSwitch, lineNumberString, ++i);
            drawLinenumAndFinder(&lineNumberString, &c, AllLineLength);
            j = 0;
            nowComment = false;
            attrset(COLOR_PAIR(NOMAL));
        }
    }

    attrset(COLOR_PAIR(NOMAL));

    while (i < h-1) {
        drawInDir(finderSwitch, lineNumberString, i++);
        lineNumberString = "~\n";
        drawTildeAndFinder(&lineNumberString,  AllLineLength);
    }

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
    move(gRow, gCol+nowLineBuf+1);
    refresh(); 
}

