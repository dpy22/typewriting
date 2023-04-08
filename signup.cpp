#include "signup.h"
#include "ui_signup.h"
#include "mainwindow.h"
#include "signin.h"
#include <QStringLiteral>
#include <QByteArray>
#include <QDebug>

Signup::Signup(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Signup)
{
    ui->setupUi(this);
    resize(600,500);
    setWindowTitle("注册");
    setWindowIcon(QIcon(":/new/prefix1/images/logo.jpg"));
}

Signup::~Signup()
{
    delete ui;
}

//返回登录按钮
void Signup::on_btn_return_clicked()
{
    Signin *sign = new Signin;
    sign->show();
    this->close();
}

//注册按钮
void Signup::on_pushButton_2_clicked()
{
    sqlite_Init();
    QString username = ui->lineEdit_username->text();
    QString password = ui->lineEdit_passwd->text();
    QString surepass = ui->lineEdit_surepasswd->text();

    //用户名base64编码
    QByteArray uidb64 = username.toLocal8Bit().toBase64();
    QString name = QString::fromLocal8Bit(uidb64);

    //密码base64编码
    QByteArray pwdb64 = password.toLocal8Bit().toBase64();
    QString pass = QString::fromLocal8Bit(pwdb64);

    //判断密码是否一致
    if(password == surepass)
    {
        //QString sql=QString("insert into user(username,password) values('%1','%2');")
        //        .arg(name).arg(pass);
        //创建执行语句对象
        QSqlQuery query;
        query.prepare("insert into user(username,password,speed,prompt) "
                      "values (?,?,?,?)");
        query.addBindValue(name);
        query.addBindValue(pass);
        query.addBindValue(NULL);
        query.addBindValue(NULL);
        //query.exec();
        //判断执行结果
        if(!query.exec())
        {
            qDebug()<<"insert into error";
            QMessageBox::information(this,QStringLiteral("注册认证"),QStringLiteral("注册失败！"));
        }
        else
        {
            qDebug()<<"insert into success";
            QMessageBox::information(this,QStringLiteral("注册认证"),QStringLiteral("注册成功！"));
            Signin *sign = new Signin;
            sign->show();
            this->close();
        }

    }else{
        QMessageBox::information(this,QStringLiteral("注册认证"),QStringLiteral("两次密码输入不一致"));
    }
}
