#ifndef SIGNUP_H
#define SIGNUP_H
#include <QWidget>

namespace Ui {
class Signup;
}

class Signup : public QWidget
{
    Q_OBJECT

public:
    Signup(QWidget *parent = nullptr);
    ~Signup();

private slots:
    void on_pushButton_2_clicked();//注册操作
    void on_btn_return_clicked();//返回登录界面操作

private:
    Ui::Signup *ui;
};

#endif // SIGNUP_H
