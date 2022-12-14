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

                for (; gBuf[++gIndex - 1] != '\n';);
                gIndex += gCol;

                for (int i = gIndex-gCol; i < gIndex; i++) {
                    if (gBuf[i] == '\t')
                        gIndex -= 3; 
                }

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
                for (;gBuf[--gIndex] != '\n';);

                /*
                gIndex ++;//= gCol+1;
                for (int i = 0; i < gCol; i++) {// - gCol; i < gIndex; i++) {
                    gIndex++;
                    if (gBuf[i+gIndex] == '\t') gIndex -= 3;
                }*/

                gIndex += gCol+1;
                for (int i = gIndex - gCol; i < gIndex; i++) {
                    if (gBuf[i] == '\t') gIndex -= 3;
                }

                for (;gIndex < tmp_gIndex; --moveDiff)
                    visualData.push_back(gBuf[tmp_gIndex--]);

                visualBuf.insert(visualBuf.size(), visualData);

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
					//reverse(yankBuf.begin(), yankBuf.end());
                }
                redraw();
                return;
            }

            case 'b': {
                //gIndex++;
                while (!isspace(gBuf[gIndex])) {
                    (--moveDiff > 0) ? visualBuf.resize(visualBuf.size() - 1)
                                     : visualBuf.push_back(gBuf[gIndex]);
                    (gIndex > 0) ? gIndex-- : 0;
                }

                break;
            }
                      /*
                       */

            case kESC:
                break;
        }
        visualEnd = gIndex;
    }
    nowMode = NOMAL_M;
}

