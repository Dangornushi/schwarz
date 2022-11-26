#include <string>

extern void finder(); 
extern void drawLinenumAndFinder(std::string *lineNumberString, int *c, const int AllLineLength);
extern void drawTildeAndFinder(std::string *lineNumberString, const int AllLineLength);
extern void drawInDir(const bool finderSwitch, const std::string lineNumberString, const int index);
