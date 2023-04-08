#ifndef TEXT_H
#define TEXT_H

#include <iostream>
#include <string>
#include <ctime>
#include <string>
using namespace std;

class Text
{
private:
    wstring originalTxt;//初始化Text类对象的
    wstring py_input;//截取某一位后转化格式输入到转拼音函数
    wstring user_input;//用户输入的文本
    int txtLength;//所有字符数

public:
    Text(wstring originalTxt)
     : originalTxt(originalTxt){}
    string findPy(int a); //核心功能函数
    int get_txtLength() { return txtLength; }
};

#endif // TEXT_H
