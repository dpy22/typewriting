#include "mainwindow.h"
#include "mytextedit.h"
#include "text.h"
#include "ui_mainwindow.h"
#include "signin.h"
#include <QAction>
#include <QPushButton>
#include <QIcon>
#include <QString>
#include <QFileDialog>
#include <QFile>
#include <QElapsedTimer>
#include <QTimer>
#include <QScrollBar>
#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>

QString stdWToQString(const std::wstring &str);
std::wstring qToStdWString(const QString &str);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("小鹤打字通");//窗口标题
    setWindowIcon(QIcon(":/new/prefix1/images/logo.jpg"));//窗口图标
    createActions();
    createMenus();
    displayTime();
    setScroll();
    this->releaseKeyboard();
    connect(ui->textEdit_2,&MyTextEdit::emit_correct,this,&MainWindow::changeCorrectLcd);
    resetButton();

    //实时纠错初始化
    redfmt.setForeground(QColor("red"));// 红色格式实现
    blackfmt.setForeground(QColor("black"));//黑色格式实现
    cursor = ui->textEdit_2->textCursor();//获取文本光标

    //显示助记提示
    connect(ui->textEdit_2,&MyTextEdit::textChanged,this,&MainWindow::findPrompt);
}

MainWindow::~MainWindow()//析构函数
{
    delete ui;
}

void MainWindow::createActions()//初始化动作函数
{
    sqlite_Init();

    defaultAction=new QAction(tr("从剪贴板导入"),this);
    connect(defaultAction,&QAction::triggered,this,&MainWindow::defaultFile);

    newAction=new QAction(tr("从文件导入"),this);
    connect(newAction,&QAction::triggered,this,&MainWindow::newFile);

    displaySpeedAction=new QAction(tr("显示速度信息"),this);
    displaySpeedAction->setCheckable(true);

    //根据数据库中的记录初始化软件设置
    QString sql=QString("select speed,prompt from user where id='%1'").arg(id);
    QSqlQuery query(sql);
    query.exec();
    query.next();
    QString sd=query.value(0).toString();
    QString pt=query.value(1).toString();
    qDebug()<<"sd:"<<sd;
    if(sd=="N")
    {
        displaySpeedAction->setChecked(false);
        displaySpeed(false);
    }
    else
    {
        displaySpeedAction->setChecked(true);
        displaySpeed(true);
    }
    connect(displaySpeedAction,&QAction::toggled,this,&MainWindow::displaySpeed);

    displayPromptAction=new QAction(tr("显示键盘提示"),this);
    displayPromptAction->setCheckable(true);
    qDebug()<<"pt:"<<pt;
    if(pt=="N")
    {
        displayPromptAction->setChecked(false);
        displayPrompt(false);
    }
    else
    {
        displayPromptAction->setChecked(true);
        displayPrompt(true);
    }
    //可通过此语句设置复选框初始值
    connect(displayPromptAction,&QAction::triggered,this,&MainWindow::displayPrompt);

    helpAction=new QAction(tr("打开帮助文档"),this);
    connect(helpAction,&QAction::triggered,this,&MainWindow::help);
}

void MainWindow::createMenus()//初始化菜单栏函数
{
    auto *fileMenu = new MyMenu;
    fileMenu->setTitle(tr("文件"));
    menuBar()->addMenu(fileMenu);
    fileMenu->addAction(defaultAction);
    fileMenu->addSeparator();
    fileMenu->addAction(newAction);
    fileMenu->addSeparator();

    auto *optionMenu=new MyMenu;
    optionMenu->setTitle(tr("功能"));
    menuBar()->addMenu(optionMenu);
    optionMenu->addAction(displaySpeedAction);
    optionMenu->addSeparator();
    optionMenu->addAction(displayPromptAction);
    optionMenu->addSeparator();
    optionMenu->exec();

    QMenu *helpMenu=new QMenu;
    helpMenu->setTitle(tr("帮助"));
    menuBar()->addMenu(helpMenu);
    helpMenu->addAction(helpAction);
    helpMenu->addSeparator();
}

void MainWindow::defaultFile()//从剪贴板导入文本函数
{
    QClipboard *clipboard = QApplication::clipboard();
    QString defaultTxt = clipboard->text();
    this->ui->textEdit->setText(defaultTxt);//这里的File_edit就是之前声明的Textedit控件的名字
    ori=qToStdWString(defaultTxt);
    findPrompt();
}

void MainWindow::newFile()//导入用户文本槽函数
{
    QString filePath=QFileDialog::getOpenFileName(this, tr("导入用户文本"),
                                             "C:/",
                                             tr("List files(*.txt *.php *.dpl *.m3u *.m3u8 *.xspf );;All files (*.*)"));
    QFile file(filePath);//file即为取到地址的文件
    file.open(QIODevice::ReadOnly | QIODevice::Text);//只读方式，文本格式
    QTextStream input(&file); //读出文件
    input.setCodec("utf-8");
    QString defaultTxt;
    while(!input.atEnd())
    {
        defaultTxt = defaultTxt + input.readLine() + "\n";
    }
    file.close();
    this->ui->textEdit->setText(defaultTxt);//这里的textedit就是之前声明的Textedit控件的名字
    ori=qToStdWString(defaultTxt);//进行格式转换
    findPrompt();
}

void MainWindow::displaySpeed(bool sta)//显示速度信息槽函数
{
    sqlite_Init();
    if(sta)
    {
        ui->speedArea->show();
        QString sql=QString("update user set speed = 'Y' WHERE id ='%1'").arg(id);
        QSqlQuery query(sql);
        query.exec();
    }
    else
    {
        ui->speedArea->hide();
        QString sql=QString("update user set speed = 'N' WHERE id ='%1'").arg(id);
        QSqlQuery query(sql);
        query.exec();
    }
}

void MainWindow::displayPrompt(bool sta)//显示键盘提示槽函数
{
    sqlite_Init();
    if(sta)
    {
        ui->promptArea_1->show();
        ui->promptArea_2->show();
        ui->promptArea_3->show();
        QString sql=QString("update user set prompt = 'Y' WHERE id ='%1'").arg(id);
        QSqlQuery query(sql);
        query.exec();
    }
    else
    {
        ui->promptArea_1->hide();
        ui->promptArea_2->hide();
        ui->promptArea_3->hide();
        QString sql=QString("update user set prompt = 'N' WHERE id ='%1'").arg(id);
        QSqlQuery query(sql);
        query.exec();
    }
}

void MainWindow::help()
{
    QDesktopServices::openUrl(QUrl("https://www.flypy.com"));
}

void MainWindow::findPrompt()
{
    if(!ui->textEdit->document()->isEmpty()&&my.length()<ori.length()&&!ui->textEdit_2->document()->isEmpty())
    {
        int i=my.length();
        //std::wcout.imbue(std::locale("chs"));
        //std::wcin.imbue(std::locale("chs"));
        setlocale(LC_ALL, "chs");//设置地域
        Text a(ori);
        QString qstring = QString(QString::fromLocal8Bit(a.findPy(i).c_str()));
        setButtonColors(qstring);
    }
    else if(ui->textEdit_2->document()->isEmpty()&&!ui->textEdit->document()->isEmpty())
    {
        Text a(ori);
        QString qstring = QString(QString::fromLocal8Bit(a.findPy(0).c_str()));
        setButtonColors(qstring);
    }
}

void MainWindow::setButtonColors(QString py)//按钮提示函数
{
    if(py=="aa"){
        resetButton();
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }
    else if(py=="ai"){
        resetButton();
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }
    else if(py=="an"){
        resetButton();
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }
    else if(py=="ah"){
        resetButton();
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ao"){
        resetButton();
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ba"){
        resetButton();
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="bd"){
        resetButton();
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="bj"){
        resetButton();
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="bh"){
        resetButton();
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="bc"){
        resetButton();
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="bw"){
        resetButton();
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->WButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="bf"){
        resetButton();
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="bg"){
        resetButton();
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="bi"){
        resetButton();
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="bm"){
        resetButton();
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="bn"){
        resetButton();
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="bp"){
        resetButton();
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="bb"){
        resetButton();
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }
    else if(py=="bk"){
        resetButton();
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="bo"){
        resetButton();
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="bu"){
        resetButton();
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ca"){
        resetButton();
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="cd"){
        resetButton();
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="cj"){
        resetButton();
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ch"){
        resetButton();
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="cc"){
        resetButton();
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }
    else if(py=="ce"){
        resetButton();
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->EButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="cg"){
        resetButton();
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ia"){
        resetButton();
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="id"){
        resetButton();
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ij"){
        resetButton();
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ih"){
        resetButton();
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ic"){
        resetButton();
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ie"){
        resetButton();
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->EButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="if"){
        resetButton();
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ig"){
        resetButton();
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ii"){
        resetButton();
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }
    else if(py=="is"){
        resetButton();
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="iz"){
        resetButton();
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="iu"){
        resetButton();
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ik"){
        resetButton();
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ir"){
        resetButton();
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="il"){
        resetButton();
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="iv"){
        resetButton();
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="iy"){
        resetButton();
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="io"){
        resetButton();
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ci"){
        resetButton();
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="cs"){
        resetButton();
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="cz"){
        resetButton();
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="cu"){
        resetButton();
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="cr"){
        resetButton();
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="cv"){
        resetButton();
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="cy"){
        resetButton();
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="co"){
        resetButton();
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="da"){
        resetButton();
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="dd"){
        resetButton();
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }
    else if(py=="dj"){
        resetButton();
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="dh"){
        resetButton();
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="dc"){
        resetButton();
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="de"){
        resetButton();
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->EButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="dg"){
        resetButton();
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="di"){
        resetButton();
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="dm"){
        resetButton();
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="dn"){
        resetButton();
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="dp"){
        resetButton();
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="dk"){
        resetButton();
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="dq"){
        resetButton();
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ds"){
        resetButton();
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="dz"){
        resetButton();
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="du"){
        resetButton();
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="dr"){
        resetButton();
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="dv"){
        resetButton();
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="dy"){
        resetButton();
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="do"){
        resetButton();
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ee"){
        resetButton();
        ui->EButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }
    else if(py=="en"){
        resetButton();
        ui->EButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="er"){
        resetButton();
        ui->EButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="fa"){
        resetButton();
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="fj"){
        resetButton();
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="fh"){
        resetButton();
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="fw"){
        resetButton();
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->WButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ff"){
        resetButton();
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }
    else if(py=="fg"){
        resetButton();
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="fo"){
        resetButton();
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="fz"){
        resetButton();
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="fu"){
        resetButton();
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ga"){
        resetButton();
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="gd"){
        resetButton();
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="gj"){
        resetButton();
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="gh"){
        resetButton();
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="gc"){
        resetButton();
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ge"){
        resetButton();
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->EButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="gf"){
        resetButton();
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="gg"){
        resetButton();
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }
    else if(py=="gs"){
        resetButton();
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="gz"){
        resetButton();
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="gu"){
        resetButton();
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="gx"){
        resetButton();
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="gk"){
        resetButton();
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="gr"){
        resetButton();
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="gl"){
        resetButton();
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="gv"){
        resetButton();
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="go"){
        resetButton();
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ha"){
        resetButton();
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="hd"){
        resetButton();
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="hj"){
        resetButton();
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="hh"){
        resetButton();
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }
    else if(py=="hc"){
        resetButton();
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="he"){
        resetButton();
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->EButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="hw"){
        resetButton();
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->WButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="hf"){
        resetButton();
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="hg"){
        resetButton();
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="hs"){
        resetButton();
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="hz"){
        resetButton();
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="hu"){
        resetButton();
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="hx"){
        resetButton();
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="hk"){
        resetButton();
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="hr"){
        resetButton();
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="hl"){
        resetButton();
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="hv"){
        resetButton();
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="hy"){
        resetButton();
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ho"){
        resetButton();
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ji"){
        resetButton();
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="jx"){
        resetButton();
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="jm"){
        resetButton();
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="jl"){
        resetButton();
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="jn"){
        resetButton();
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="jp"){
        resetButton();
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="jb"){
        resetButton();
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="jk"){
        resetButton();
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="js"){
        resetButton();
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="jq"){
        resetButton();
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ju"){
        resetButton();
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="jr"){
        resetButton();
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="jt"){
        resetButton();
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="jy"){
        resetButton();
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ka"){
        resetButton();
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="kd"){
        resetButton();
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="kj"){
        resetButton();
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="kh"){
        resetButton();
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="kc"){
        resetButton();
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ke"){
        resetButton();
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->EButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="kf"){
        resetButton();
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="kg"){
        resetButton();
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ks"){
        resetButton();
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="kz"){
        resetButton();
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ku"){
        resetButton();
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="kx"){
        resetButton();
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="kk"){
        resetButton();
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }
    else if(py=="kr"){
        resetButton();
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="kl"){
        resetButton();
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="kv"){
        resetButton();
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ky"){
        resetButton();
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ko"){
        resetButton();
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="la"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ld"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="lj"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="lh"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="lc"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="le"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->EButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="lw"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->WButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="lg"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="li"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="lx"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="lm"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ll"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }
    else if(py=="ln"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="lp"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="lb"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="lk"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="lq"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ls"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="lz"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="lu"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="lv"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="lr"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="lt"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ly"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="lo"){
        resetButton();
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ma"){
        resetButton();
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="md"){
        resetButton();
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="mj"){
        resetButton();
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="mh"){
        resetButton();
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="mc"){
        resetButton();
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="me"){
        resetButton();
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->EButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="mw"){
        resetButton();
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->WButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="mf"){
        resetButton();
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="mg"){
        resetButton();
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="mi"){
        resetButton();
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="mm"){
        resetButton();
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }
    else if(py=="mn"){
        resetButton();
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="mp"){
        resetButton();
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="mb"){
        resetButton();
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="mk"){
        resetButton();
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="mq"){
        resetButton();
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="mo"){
        resetButton();
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="mz"){
        resetButton();
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="mu"){
        resetButton();
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="na"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="nd"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="nj"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="nh"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="nc"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ne"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->EButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="nw"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->WButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="nf"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ng"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ni"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="nm"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="nl"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="nn"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }
    else if(py=="np"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="nb"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="nk"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="nq"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ns"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="nu"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="nv"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="nr"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="nt"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="no"){
        resetButton();
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="oo"){
        resetButton();
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }
    else if(py=="ou"){
        resetButton();
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="pa"){
        resetButton();
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="pd"){
        resetButton();
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="pj"){
        resetButton();
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ph"){
        resetButton();
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="pc"){
        resetButton();
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="pw"){
        resetButton();
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->WButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="pf"){
        resetButton();
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="pg"){
        resetButton();
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="pi"){
        resetButton();
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="pm"){
        resetButton();
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="pn"){
        resetButton();
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="pp"){
        resetButton();
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }
    else if(py=="pb"){
        resetButton();
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="pk"){
        resetButton();
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="po"){
        resetButton();
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="pz"){
        resetButton();
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="pu"){
        resetButton();
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="qi"){
        resetButton();
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="qx"){
        resetButton();
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="qm"){
        resetButton();
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ql"){
        resetButton();
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="qn"){
        resetButton();
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="qp"){
        resetButton();
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="qb"){
        resetButton();
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="qk"){
        resetButton();
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="qs"){
        resetButton();
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="qq"){
        resetButton();
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="qu"){
        resetButton();
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="qr"){
        resetButton();
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="qt"){
        resetButton();
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="qy"){
        resetButton();
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="rj"){
        resetButton();
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="rh"){
        resetButton();
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="rc"){
        resetButton();
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="re"){
        resetButton();
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->EButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="rf"){
        resetButton();
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="rg"){
        resetButton();
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ri"){
        resetButton();
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="rs"){
        resetButton();
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="rz"){
        resetButton();
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ru"){
        resetButton();
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="rr"){
        resetButton();
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }
    else if(py=="rv"){
        resetButton();
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ry"){
        resetButton();
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ro"){
        resetButton();
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="sa"){
        resetButton();
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="sd"){
        resetButton();
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="sj"){
        resetButton();
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="sh"){
        resetButton();
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="sc"){
        resetButton();
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="se"){
        resetButton();
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->EButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="sf"){
        resetButton();
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="sg"){
        resetButton();
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ua"){
        resetButton();
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ud"){
        resetButton();
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="uj"){
        resetButton();
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="uh"){
        resetButton();
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="uc"){
        resetButton();
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ue"){
        resetButton();
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->EButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="uf"){
        resetButton();
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ug"){
        resetButton();
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }
    else if(py=="ui"){
        resetButton();
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="uz"){
        resetButton();
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="uu"){resetButton();
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="ux"){
        resetButton();
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="uk"){
        resetButton();
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="ur"){
        resetButton();
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ul"){
        resetButton();
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="uv"){
        resetButton();
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="uy"){
        resetButton();
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }
    else if(py=="uo"){
        resetButton();
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="si"){
        resetButton();
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ss"){
        resetButton();
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="sz"){
        resetButton();
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="su"){
        resetButton();
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="sr"){
        resetButton();
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="sv"){
        resetButton();
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="sy"){
        resetButton();
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="so"){
        resetButton();
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="ta"){
        resetButton();
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="td"){
        resetButton();
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="tj"){
        resetButton();
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="th"){
        resetButton();
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="tc"){
        resetButton();
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="te"){
        resetButton();
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->EButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="tg"){
        resetButton();
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ti"){
        resetButton();
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="tm"){
        resetButton();
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="tn"){
        resetButton();
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="tp"){
        resetButton();
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="tk"){
        resetButton();
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="ts"){
        resetButton();
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="tz"){
        resetButton();
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="tu"){
        resetButton();
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="tr"){
        resetButton();
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="tv"){
        resetButton();
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ty"){
        resetButton();
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="to"){
        resetButton();
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="wa"){
        resetButton();
        ui->WButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="wd"){
        resetButton();
        ui->WButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="wj"){
        resetButton();
        ui->WButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="wh"){
        resetButton();
        ui->WButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="ww"){
        resetButton();
        ui->WButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->WButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }
    else if(py=="wf"){
        resetButton();
        ui->WButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="wg"){
        resetButton();
        ui->WButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="wo"){
        resetButton();
        ui->WButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="wu"){
        resetButton();
        ui->WButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="xi"){
        resetButton();
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="xx"){
        resetButton();
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="xm"){
        resetButton();
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->MButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="xl"){
        resetButton();
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="xn"){
        resetButton();
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->NButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="xp"){
        resetButton();
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->PButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="xb"){
        resetButton();
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="xk"){
        resetButton();
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="xs"){
        resetButton();
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="xq"){
        resetButton();
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->QButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="xu"){
        resetButton();
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="xr"){
        resetButton();
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="xt"){
        resetButton();
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="xy"){
        resetButton();
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="ya"){
        resetButton();
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="yj"){
        resetButton();
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="yh"){
        resetButton();
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="yc"){
        resetButton();
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="ye"){
        resetButton();
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->EButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="yi"){
        resetButton();
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="yb"){
        resetButton();
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->BButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="yk"){
        resetButton();
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="yo"){
        resetButton();
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }


    else if(py=="ys"){
        resetButton();
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="yz"){
        resetButton();
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="yu"){
        resetButton();
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="yr"){
        resetButton();
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
        }

    else if(py=="yt"){
        resetButton();
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->TButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="yy"){
        resetButton();
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }

    else if(py=="za"){
        resetButton();
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="zd"){
        resetButton();
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="zj"){
        resetButton();
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="zh"){
        resetButton();
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="zc"){
        resetButton();
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="ze"){
        resetButton();
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->EButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="zw"){
        resetButton();
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->WButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="zf"){
        resetButton();
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="zg"){
        resetButton();
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="va"){
        resetButton();
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->AButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="vd"){
        resetButton();
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->DButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="vj"){
        resetButton();
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->JButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="vh"){
        resetButton();
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->HButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="vc"){
        resetButton();
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->CButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="ve"){
        resetButton();
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->EButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="vf"){
        resetButton();
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->FButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="vg"){
        resetButton();
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->GButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="vi"){
        resetButton();
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="vs"){
        resetButton();
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="vz"){
        resetButton();
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="vu"){
        resetButton();
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="vx"){
        resetButton();
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->XButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="vk"){
        resetButton();
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->KButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="vr"){
        resetButton();
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="vl"){
        resetButton();
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->LButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="vv"){
        resetButton();
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }

    else if(py=="vy"){
        resetButton();
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="vo"){
        resetButton();
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="zi"){
        resetButton();
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->IButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else if(py=="zs"){
        resetButton();
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->SButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="zz"){
        resetButton();
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
    }

    else if(py=="zu"){
        resetButton();
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->UButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="zr"){
        resetButton();
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->RButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="zv"){
        resetButton();
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->VButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="zy"){
        resetButton();
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->YButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }

    else if(py=="zo"){
        resetButton();
        ui->ZButton->setStyleSheet("QPushButton{background-color:rgb(170,200,50)}");
        ui->OButton->setStyleSheet("QPushButton{background-color:rgb(0,191,255)}");
    }
    else{
        resetButton();
    }
}

void MainWindow::resetButton()//重置按钮提示函数
{
    ui->QButton->setStyleSheet("");
    ui->WButton->setStyleSheet("");
    ui->EButton->setStyleSheet("");
    ui->RButton->setStyleSheet("");
    ui->TButton->setStyleSheet("");
    ui->YButton->setStyleSheet("");
    ui->UButton->setStyleSheet("");
    ui->IButton->setStyleSheet("");
    ui->OButton->setStyleSheet("");
    ui->PButton->setStyleSheet("");
    ui->AButton->setStyleSheet("");
    ui->SButton->setStyleSheet("");
    ui->DButton->setStyleSheet("");
    ui->FButton->setStyleSheet("");
    ui->GButton->setStyleSheet("");
    ui->HButton->setStyleSheet("");
    ui->JButton->setStyleSheet("");
    ui->KButton->setStyleSheet("");
    ui->LButton->setStyleSheet("");
    ui->ZButton->setStyleSheet("");
    ui->XButton->setStyleSheet("");
    ui->CButton->setStyleSheet("");
    ui->VButton->setStyleSheet("");
    ui->BButton->setStyleSheet("");
    ui->NButton->setStyleSheet("");
    ui->MButton->setStyleSheet("");

}

void MainWindow::displayTime()//时间脉冲函数
{
    thisCondition=true;
    lastCondition=true;
    timer=new QTimer(this);
    timer_2=new QTimer(this);
    elapsedtimer=new QElapsedTimer;
    connect(timer,&QTimer::timeout,this,&MainWindow::timerMessage);
    connect(timer_2,&QTimer::timeout,this,&MainWindow::speedMessage);
    connect(timer,&QTimer::timeout,this,&MainWindow::textFormat);
    timer->start(50);
    timer_2->start(2000);
}

void MainWindow::setScroll()//文本框同步函数
{
    QScrollBar *QSB=ui->textEdit->verticalScrollBar();
    QScrollBar *QSB_2=ui->textEdit_2->verticalScrollBar();

    connect(QSB,&QScrollBar::valueChanged,this,&MainWindow::upScroll);//valueChanged()带参数
    connect(QSB_2,&QScrollBar::valueChanged,this,&MainWindow::downScroll);
}

void MainWindow::upScroll()//下与上同步槽函数
{
    QScrollBar *QSB=ui->textEdit->verticalScrollBar();
    QScrollBar *QSB_2=ui->textEdit_2->verticalScrollBar();
    QSB_2->setValue(QSB->value());
}

void MainWindow::downScroll()//上与下同步槽函数
{
    QScrollBar *QSB=ui->textEdit->verticalScrollBar();
    QScrollBar *QSB_2=ui->textEdit_2->verticalScrollBar();
    QSB->setValue(QSB_2->value());
}

void MainWindow::timerMessage()//输入框状态检测和时间显示函数
{
    lastCondition=thisCondition;
    thisCondition=ui->textEdit_2->document()->isEmpty();//记录状态改变
    if(lastCondition==true&&thisCondition==false)//由空到非空时开始计时
    {
        elapsedtimer->start();
        middle=elapsedtimer->elapsed();
        second=(middle/1000)%60;
        minute=(middle/1000)/60;
        timestr=timestr.sprintf("%02d:%02d",minute,second);
        ui->timeLcdNumber->display(timestr);
    }
    else if(lastCondition==true&&thisCondition==true)//始终为空时显示为0
    {
        ui->timeLcdNumber->display(0);
    }
    else if(lastCondition==false&&thisCondition==false)//始终为非空时动态显示时间
    {
        middle=elapsedtimer->elapsed();
        second=(middle/1000)%60;
        minute=(middle/1000)/60;
        timestr=timestr.sprintf("%02d:%02d",minute,second);
        ui->timeLcdNumber->display(timestr);
    }
}

void MainWindow::speedMessage()//速度信息槽函数
{
    if(!ui->textEdit_2->document()->isEmpty())
    {
        QString my1=ui->textEdit_2->toPlainText();
        speed=1.0*1000*60*my1.length()/elapsedtimer->elapsed();
        speedstr=speedstr.sprintf("%.1f",speed);
        ui->speedLcdNumber->display(speedstr);
    }
    else
    {
        ui->speedLcdNumber->display(0);
    }
}

void MainWindow::changeCorrectLcd(int a)//回改信息槽函数
{
    ui->correctLcdNumber->display(a);
}

/*! Convert an std::wstring to a QString */
QString stdWToQString(const std::wstring &str)//wstring格式转QString函数
{
#ifdef _MSC_VER
    return QString::fromUtf16((const ushort *)str.c_str());
#else
    return QString::fromStdWString(str);
#endif
}

/*! Convert an QString to a std::wstring */
std::wstring qToStdWString(const QString &str)//QString格式转wstring函数
{
#ifdef _MSC_VER
    return std::wstring((const wchar_t *)str.utf16());
#else
    return str.toStdWString();
#endif
}

void MainWindow::textFormat()//实时纠错函数
{
    if(!ui->textEdit_2->document()->isEmpty()&&!ui->textEdit->document()->isEmpty())
    {
        QString my1=ui->textEdit_2->toPlainText();
        my=qToStdWString(my1);
        mistake=0;

        for(int i=0;i<=my.length()-1;i++)
        {
            if(ori[i]==my[i])
            {
                cursor.setPosition(i);
                cursor.setPosition(i+1,QTextCursor::KeepAnchor);
                cursor.mergeCharFormat(blackfmt);//光标选中的文字就用该格式显示
            }
            else if(ori[i]!=my[i])
            {
                cursor.setPosition(i);
                cursor.setPosition(i+1,QTextCursor::KeepAnchor);
                cursor.mergeCharFormat(redfmt);//光标选中的文字就用该格式显示
                ui->textEdit->mergeCurrentCharFormat(blackfmt);//textEdit使用当前的字符格式
                mistake++;
            }
        }
        ui->mistakeLcdNumber->display(mistake);
    }
}
