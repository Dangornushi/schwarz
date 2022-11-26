#include "schwarz.hpp"

extern int moveDiff;
extern string yankBuf;

template <typename T>
void slice(std::vector<T> &v, int m, int n) {
    auto first = v.cbegin() + m;
    auto last = v.cbegin() + n + 1; 
    v.erase(first, last);
}

// visualMode
void visualMode() {
    nowMode = VISUAL_M;
    moveDiff = 0;
    int base_gIndex;
    string visualBuf;

    yankBuf.clear();

    base_gIndex = gIndex;
    visualStart = gIndex;

    for (int ch; (ch = getch()) != kESC; display()) {

        switch (ch) {
            case 'h': {
                (--moveDiff > 0) ? visualBuf.resize(visualBuf.size() - 1)
                                 : visualBuf.push_back(gBuf[gIndex]);

                if (gIndex > 0) gIndex--;
                break;
            }

            case 'j': {
                int tmp_gIndex = gIndex;

                for (; gBuf[++gIndex - 1] != '\n';) {
                    if (gBuf[gIndex + 2] == '\t') gCol -= 3;
                }
                gIndex += gCol;

                int i = 0;
                string visualData;

                move (gRow+1, gCol);
                for (;tmp_gIndex < gIndex; ++moveDiff, i++) {
                    visualData.push_back(gBuf[--gIndex]);
                    attrset(COLOR_PAIR(NOMAL));
                    printw("%c", gBuf[gIndex]);
                }

                reverse(visualData.begin(), visualData.end());
                visualBuf.insert(visualBuf.size(), visualData);

                gIndex += i;
                break;
            }

            case 'k': { 
                int tmp_gIndex = gIndex;
                string visualData;

                for (;gBuf[--gIndex] != '\n';);
                if (gBuf[gIndex-3] != '\n' && gBuf[gIndex-2] != '\n') {
                    for (; gBuf[--gIndex] != '\n';) {
                        if (gBuf[gIndex] == '\t') gCol -= 3;
                    }

                    gIndex += gCol;
                } else
                    --gIndex;

                int now_gIndex = gIndex+1;
                for (;tmp_gIndex-1 > gIndex; --moveDiff)
                    visualData.push_back(gBuf[++gIndex]);

                visualBuf.insert(0, visualData);

                gIndex = now_gIndex;

                redraw();
                break;
            }


            case 'l': {
                (++moveDiff >= 0) ? visualBuf.push_back(gBuf[gIndex])
                                  : visualBuf.resize(visualBuf.size() - 1);
                gIndex++;
                break;
            }

            case 'y': {
                yankBuf = visualBuf;
                yankBuf.push_back(gBuf[gIndex]);
                return;
            }

            case 'd': {
                yankBuf = visualBuf;
                yankBuf.push_back(gBuf[gIndex]);
                
                if (moveDiff > 0) {
                    slice(gBuf, base_gIndex, gIndex);
                    gIndex = base_gIndex;
                } else  {
                    slice(gBuf, gIndex, base_gIndex);
					reverse(yankBuf.begin(), yankBuf.end());
                }
                redraw();
                return;
            }

            case kESC:
                break;
        }
        visualEnd = gIndex;
    }
    nowMode = NOMAL_M;
}

