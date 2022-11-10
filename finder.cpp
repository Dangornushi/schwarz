#include "schwarz.hpp"

namespace fs = std::__fs::filesystem;

const fs::directory_iterator end;

void printDirectoryEntry(const fs::directory_entry &entry, int *maxNameLength) {
    string fileOrDir = entry.path().string();

    (fileOrDir.size() > *maxNameLength) ? *maxNameLength = fileOrDir.length() : 0;
    
    if (entry.is_directory())
        // directory;
        finderData.push_back(fileOrDir);

    else if (entry.is_regular_file())
        // file;
        finderData.push_back("- " + fileOrDir);
}

void finder() {
    if (finderSwitch) {
        finderSwitch = false;
        display();
    }
    else {
        int maxNameLength = 0;

        finderData.clear();

        try {
            fs::directory_iterator it{"."};
            finderData.push_back("* Root *");

            for (const fs::directory_entry& entry : it)
                printDirectoryEntry(entry, &maxNameLength);
        }
        catch (const fs::filesystem_error& e) {
            quit();
            printw("ディレクトリ内のファイルを参照する際に問題が発生しました\n");
        }

        string lengthBuf = "===--- Finder ---===";

        for (int i = lengthBuf.length(); i < (maxNameLength%2 == 1 ? maxNameLength : maxNameLength+1); i++) 
            lengthBuf = " " + lengthBuf + " ";
        
        finderData.insert(finderData.begin(), lengthBuf);

        finderSwitch = true;
        display();
    }
}

