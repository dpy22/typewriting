#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "mymenu.h"
#include "mytextedit.h"
#include "text.h"
#include <QElapsedTimer>

class QAction;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow//主界面的生成和主要功能的实现
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void defaultFile();//从剪贴板导入文本槽
    void newFile();//从文件导入槽
    void displaySpeed(bool sta);//显示速度槽
    void displayPrompt(bool sta);//显示提示槽
    void help();//打开帮助文档槽
    void timerMessage();//时间消息槽，每隔一段时间检测输入框状态，决定计时器时间
    void changeCorrectLcd(int a);//更新回改次数槽
    void textFormat();//实时纠错槽
    void upScroll();//下与上同步槽
    void downScroll();//上与下同步槽
    void speedMessage();//速度消息槽，调整速度信息
    void findPrompt();//连接Text类槽

private:
    Ui::MainWindow *ui;
    void createActions();//创建各项动作
    void createMenus();//创建各项菜单
    void setButtonColors(QString py);//设定按钮颜色
    void resetButton();//重置按钮颜色
    void setScroll();//设置滚动条
    void displayTime();//时间控制函数

    MyMenu *fileMenu;//文件菜单
    MyMenu *optionMenu;//功能菜单

    QAction *defaultAction;//从剪贴板导入文本
    QAction *newAction;//从文件导入
    QAction *displaySpeedAction;//显示速度信息
    QAction *displayPromptAction;//显示键盘提示
    QAction *helpAction;//打开帮助文档

    //和时间有关的一切变量
    QTimer *timer;//50ms循环定时器
    QTimer *timer_2;//2000ms循环定时器
    QElapsedTimer *elapsedtimer;//计时器，从开始输入计时，显示开始时间
    bool lastCondition;//上次检测时的输入状态
    bool thisCondition;//本次检测时的输入状态
    int middle=0;//格式转换中间值
    int minute;//格式转换后的分钟值
    int second;//格式转换后的秒钟值
    float speed=0;//速度信息
    QString timestr;//时间字符串
    QString speedstr;//速度字符串

    //和纠错和格式有关的一切变量
    std::wstring ori=L"";//参考文本
    std::wstring my=L"";//输入文本
    QTextCursor cursor;//隐式光标
    QTextCharFormat redfmt;//红色文本格式
    QTextCharFormat blackfmt;//黑色文本格式
    int mistake=0;//错字数量
};
#endif // MAINWINDOW_H
