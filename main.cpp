//=================================
/*!
 *  小鹤打字通 1.0.0版
 *  作者：段培耀 包戴宁
 *  编译器：Desktop_Qt_5_15_2_MinGW_64_bit
 *  2022年12月
 */
//=================================

#include "mainwindow.h"
#include "signin.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Signin sign;
    sign.show();
    return a.exec();
}
