#include "schwarz.hpp"

short R;
short G;
short B;

int RGB_buf = 4;//215686274509802;

#define BACK 0

int color_gen(int r, int g, int b, int color) {
    init_color(color, r*RGB_buf, g*RGB_buf, b*RGB_buf);
    return color;
}

void backChange() {
    // Air line
    init_pair(NOMAL_MODE, NOMAL, color_gen(129, 170, 230, NOMAL_MODE));
    init_pair(INSERT_MODE, NOMAL, color_gen(180, 142, 173, INSERT_MODE));
    init_pair(COMMAND_MODE, NOMAL, color_gen(160, 190, 140, INSERT_MODE));

    color_gen(46, 52, 64, BACK);
    color_gen(76, 86, 106, LINENUM);
    color_gen(94, 129, 172, RESERVED);
    color_gen(141, 188, 187, FUNCTION);
    color_gen(129, 161, 193, TYPE);
    color_gen(216, 222, 233, NOMAL);
    color_gen(97, 151, 162, OP);
    color_gen(142, 89, 114, NUMBER);
    color_gen(71, 85, 92, COMMENT);
    color_gen(136, 192, 208, MACRO);
    color_gen(163, 190, 140, SP_RESERVED);

    init_pair(STATUS, NOMAL, NOMAL_MODE); 
    init_pair(COMMANDLINE, NOMAL, BACK); 
    init_pair(NOMAL_MODE, BACK, NOMAL_MODE);
    init_pair(INSERT_MODE, NOMAL, INSERT_MODE);
    init_pair(COMMAND_MODE, NOMAL, COMMAND_MODE);

    int back = BACK;

    init_pair(LINE, LINENUM, back);
    init_pair(NOMAL, NOMAL, back);
    init_pair(TYPE, TYPE, back);
    init_pair(MACRO, MACRO, back);
    init_pair(NUMBER, NUMBER, back);
    init_pair(PARENTHESES, 0x4B, back);
    init_pair(BRACKETS, 0xE2, back);
    init_pair(VARIABLE, 0x74, back);
    init_pair(RESERVED, RESERVED, back);
    init_pair(OP, OP, back);
    init_pair(SUBWIN, 0x69, 0x5A); 
    init_pair(SP_RESERVED, SP_RESERVED, back);
    init_pair(COMMENT, COMMENT, back);
    init_pair(CONSECUTIVECOMMENT, COMMENT, back);
    init_pair(FUNCTION, FUNCTION, back);

}

void tokenPaint(int  *nowToken, int *tokenCounter, const int len, const int attribute) {
    if (*tokenCounter == 0) *tokenCounter = len;
    (*tokenCounter)--;
    *nowToken = attribute;
    attrset(COLOR_PAIR(attribute));
}

