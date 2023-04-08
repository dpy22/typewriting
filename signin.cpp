#include "signin.h"
#include "ui_signin.h"
#include "signup.h"
#include "mainwindow.h"
#include <QString>
#include <QStringLiteral>
#include <QByteArray>
#include <QDebug>

Signin::Signin(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Signin)
{
    ui->setupUi(this);
    resize(600,500);
    setWindowTitle("小鹤打字通");
    setWindowIcon(QIcon(":/new/prefix1/images/logo.jpg"));
    //初始化数据库
    sqlite_Init();
}

Signin::~Signin()
{
    delete ui;
}

void sqlite_Init()
{

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("user.db");
    if(!db.open())
    {
        qDebug()<<"open error";
    }
    //创建数据库
    QString createsql=QString("create table if not exists user(id integer primary key autoincrement,"
                        "username ntext unique not NULL,"
                        "password ntext not NULL,"
                        "speed ntext not NULL,"
                        "prompt ntext not NULL)");
    QSqlQuery query;
    if(!query.exec(createsql)){
        qDebug()<<"table create error";
    }
    else{
        qDebug()<<"table create success";
    }
}

int id;

void Signin::on_btn_signin_clicked()
{
    sqlite_Init();

    QString username = ui->lineEdit_username->text();
    QString password = ui->lineEdit_password->text();
    //用户名base64加密
    QByteArray uidb64 = username.toLocal8Bit().toBase64();
    QString name = QString::fromLocal8Bit(uidb64);
    //密码base64加密
    QByteArray pwdb64 = password.toLocal8Bit().toBase64();
    QString pass = QString::fromLocal8Bit(pwdb64);

    QString sql=QString("select * from user where username='%1' and password='%2'")
            .arg(name).arg(pass);
    //创建执行语句对象
    QSqlQuery query(sql);
    //判断执行结果
    if(!query.next())
    {
        qDebug()<<"Login error";
        QMessageBox::information(this,QStringLiteral("登录认证"),QStringLiteral("登录失败，用户名或密码错误"));

    }
    else
    {
        id=query.value(0).toInt();
        qDebug()<<"Login success";
        QMessageBox::information(this,QStringLiteral("登录认证"),QStringLiteral("登录成功"));
        this->close();
        MainWindow *w = new MainWindow;
        w->show();
    }
}

void Signin::on_btn_signup_clicked()
{
    this->close();
    Signup *s = new Signup;
    s->show();
}
