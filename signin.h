#ifndef SIGNIN_H
#define SIGNIN_H
#include <QWidget>
#include <QSqlDatabase>//数据驱动
#include <QSqlQuery>//数据库执行语句
#include <QMessageBox>//消息盒子

extern int id;
void sqlite_Init();

namespace Ui {
class Signin;
}

class Signin : public QWidget
{
    Q_OBJECT

public:
    Signin(QWidget *parent = nullptr);
    ~Signin();

private slots:
    void on_btn_signin_clicked();//登录操作
    void on_btn_signup_clicked();//注册操作

private:
    Ui::Signin *ui;
};

#endif // SIGNIN_H
