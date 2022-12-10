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
//    printw("%s", (*lineNumberString).c_str());
}

void drawTildeAndFinder(string *lineNumberString, const int AllLineLength) {
    attrset(COLOR_PAIR(COMMANDLINE));
    printw(" ");

    for (; AllLineLength > (*lineNumberString).size();
        (*lineNumberString).insert(0, " "))
        ;

    (*lineNumberString).insert(0, " ");
    attrset(COLOR_PAIR(LINE));
//    printw("%s", (*lineNumberString).c_str());

    printw("\n");
}

void printDirectoryEntry(const fs::directory_entry &entry, int *maxNameLength) {
    string fileOrDir = entry.path().string();//.erase(0, 2);
    fileOrDir = fileOrDir.erase(0, 2);

    if (entry.is_directory())
        finderData.push_back("D " + fileOrDir);

    else if (entry.is_regular_file())
        finderData.push_back(fileOrDir = "F " + fileOrDir);

    fileAndDirS.push_back(fileOrDir);

    (fileOrDir.length() > *maxNameLength) ? *maxNameLength = fileOrDir.length() : 0;
    dirFilesNum++;
    
}

void finderQuit() {
    windows--;
    finderSwitch = false;
    display();
}

void drawInDir(const bool finderSwitch, const string lineNumberString, const int index) {

    if (!finderSwitch) {
        return;
    }

    if (finderData.size() <= index) {

        for (int k = lineNumberString.length(); k < nowLineBuf; k++)
            printw(" ");
    } else {
        string fd = finderData[index];
        if (fd[0] == 'F')
            attrset(COLOR_PAIR(NOMAL));

        if (fd[0] == 'D')
            attrset(COLOR_PAIR(TYPE));

        printw(" %s", fd.c_str());

        int k = finderData[index].size() + lineNumberString.length();
        if (k > nowLineBuf)
            nowLineBuf = k+1;
        else {
            for (; k < nowLineBuf; k++) printw(" ");
        }
    }
    attrset(COLOR_PAIR(NOMAL));
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

//    finderData.insert(finderData.begin(), lengthBuf);

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

