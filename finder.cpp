#include "schwarz.hpp"

namespace fs = std::__fs::filesystem;
const fs::directory_iterator end;
int fCol;
int fRow;
int dirFilesNum;
void finder();
void drawFinder();
vector<string> fileAndDirS;

void drawLinenumAndFinder(string *lineNumberString, int *c, const int AllLineLength) {
    attrset(COLOR_PAIR(COMMANDLINE));
    printw(" ");

    *lineNumberString = to_string(LineStart + (*c)++) + " ";
    for (; AllLineLength > (*lineNumberString).size();
        (*lineNumberString).insert(0, " "))
        ;

    (*lineNumberString).insert(0, " ");
    attrset(COLOR_PAIR(LINE));
    printw("%s", (*lineNumberString).c_str());
}

void drawTildeAndFinder(string *lineNumberString, const int AllLineLength) {
    attrset(COLOR_PAIR(COMMANDLINE));
    printw(" ");

    for (; AllLineLength > (*lineNumberString).size();
        (*lineNumberString).insert(0, " "))
        ;

    (*lineNumberString).insert(0, " ");
    attrset(COLOR_PAIR(LINE));
    printw("%s", (*lineNumberString).c_str());
}

void printDirectoryEntry(const fs::directory_entry &entry, int *maxNameLength) {
    string fileOrDir = entry.path().string();//.erase(0, 2);

    if (entry.is_directory())
        finderData.push_back(fileOrDir);

    else if (entry.is_regular_file())
        finderData.push_back(fileOrDir = "  " + fileOrDir);

    fileAndDirS.push_back(fileOrDir);

    (fileOrDir.length() > *maxNameLength) ? *maxNameLength = fileOrDir.length() : 0;
    dirFilesNum++;
    
}

void finderQuit() {
    windows--;
    finderSwitch = false;
    display();
}

void finderCursor() {
    
    attrset(COLOR_PAIR(NOMAL));
    for (int ch;;refresh()) {
        ch = getch();

        switch(ch) {
            case 'k': {
                (fRow > 2) ? fRow-- : 0;
                move(fRow+1, 0);
                printw(" ");
                move(fRow, 0);
                printw(">");
                break;
            }

            case 'j': {
                (fRow <= dirFilesNum) ? fRow++ : 0;
                move(fRow-1, 0);
                printw(" ");
                move(fRow, 0);
                printw(">");
                break;
            }

            case '\n': {
                //gDone = false;
                globalInit();
                clear();

                gFileName = fileAndDirS[fRow-2].erase(fileAndDirS[fRow-2].find(" "), 2).c_str();  
                run();
                return;
            }

            case kCtrlF:
                finder();
                return;

            default:
                break;
        }

    }
}

void drawInDir(const bool finderSwitch, const string lineNumberString, const int index) {
    if (!finderSwitch)
        return;
    if (finderData.size() <= index) {
        for (int k = lineNumberString.length() + 1; k < nowLineBuf; k++)
            printw(" ");
    } else {
        printw("%s", finderData[index].c_str());

        for (int k = finderData[index].size() + lineNumberString.length() + 1;
             k < nowLineBuf; k++)
            printw(" ");
    }
}

void drawFinder() {
    int maxNameLength = 0;
    fCol = 0;
    fRow = 2;
    dirFilesNum = 0;

    finderData.clear();

    try {
        fs::directory_iterator it{"."};
        finderData.push_back("* Root *");

        for (const fs::directory_entry &entry : it)
            printDirectoryEntry(entry, &maxNameLength);
    } catch (const fs::filesystem_error &e) {
        quit();
        printw("ディレクトリ内のファイルを参照する際に問題が発生しました\n");
    }

    string lengthBuf = "===--- Finder ---===";

    for (int i = lengthBuf.length(); i < maxNameLength; i++)
        lengthBuf = " " + lengthBuf + " ";

    finderData.insert(finderData.begin(), lengthBuf);

    finderSwitch = true;
    display();

    move(fRow, fCol);
    refresh();
}

void finder() {
    if (finderSwitch)
        finderQuit();
    else
        drawFinder();
}

