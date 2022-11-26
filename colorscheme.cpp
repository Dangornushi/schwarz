#include "schwarz.hpp"

short R;
short G;
short B;

int RGB_buf = 4;//215686274509802;

#define BACK 0

void backChange() {
    // Air line
    init_color(BACK, 28*RGB_buf, 32*RGB_buf, 42*RGB_buf);
    init_color(LINE, 23*RGB_buf, 25*RGB_buf, 38*RGB_buf);
    init_color(COMMANDLINE, 350, 350, 400);
    init_color(NOMAL_MODE, 560, 400, 430);
    init_color(INSERT_MODE, 265, 300, 30);
    init_color(COMMAND_MODE, 250, 560, 250);
    init_color(RESERVED, 105*RGB_buf, 136*RGB_buf, 186*RGB_buf);
    init_color(TYPE, 191*RGB_buf, 148*RGB_buf, 228*RGB_buf); 
    init_color(FUNCTION, 159*RGB_buf, 149*RGB_buf, 102*RGB_buf); 
    init_color(NOMAL, 185*RGB_buf, 187*RGB_buf, 198*RGB_buf); 
    init_color(OP, 97*RGB_buf, 151*RGB_buf, 162*RGB_buf); 
    init_color(NUMBER, 142*RGB_buf, 89*RGB_buf, 114*RGB_buf); 
    init_color(COMMENT, 71*RGB_buf, 85*RGB_buf, 92*RGB_buf); 

    init_pair(STATUS, NOMAL, NOMAL_MODE); 
    init_pair(COMMANDLINE, NOMAL, COMMANDLINE); 
    init_pair(NOMAL_MODE, NOMAL, NOMAL_MODE);
    init_pair(INSERT_MODE, NOMAL, INSERT_MODE);
    init_pair(COMMAND_MODE, NOMAL, COMMAND_MODE);

    int back = BACK;

    init_pair(LINE, NOMAL, LINE);         //
    init_pair(NOMAL, NOMAL, back);        // 黒地に白文字
    init_pair(TYPE, TYPE, back);         // 黒地に水色文字
    init_pair(MACRO, 0xA3, back);
    init_pair(NUMBER, NUMBER, back);       // 黒地にオレンジ文字
    init_pair(PARENTHESES, 0x4B, back);  // 黒地に黄色文字
    init_pair(BRACKETS, 0xE2, back);     // 黒地に黄色文字
    init_pair(VARIABLE, 0x74, back);     // 黒地に黄色文字
    init_pair(RESERVED, RESERVED, back);     // 黒地にオレンジ文字
    init_pair(OP, OP, back);
    init_pair(SUBWIN, 0x69, 0x5A); 
    init_pair(SP_RESERVED, 0xA7, back);
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

