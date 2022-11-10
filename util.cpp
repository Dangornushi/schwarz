#include "schwarz.hpp"

// util
bool find(const string s, const vector<Token> v) {
    for (auto autoV : v)
        if (s == autoV.word)
            return true;
    return false;
}

bool split_token(string::iterator data, const char *word, int index) {
     for (int i = 0; i < index; i++)
        if (data[i] != word[i])
            return false;
    return true;
}

bool isChar(char data) {
    return (data >= 'a' && data <= 'z') || 
           (data >= 'A' && data <= 'Z') ||
           (data >= '0' && data <= '9') ||
           (data == '_')
               ? true
               : false;
}

void index(const string word, const vector<char> vec) {
    int v = gIndex;
    bool passd = false;

    for (;gIndex < vec.size() && !passd;)
        for (int i = 0; i<word.size()-1;i++)
            if (!(passd = ((vec[gIndex++] != word[i]) == false)))
                break;

    (vec.size() > gIndex) ? gIndex : gIndex = v;
}

bool isFunction(string::iterator data) {
    for (;data < gBuf.end();data++) {
        if (*data == '(')
            return true;
        if (*data == ' ' || !isChar(*data))
            return false;
    }
    return false;
}

