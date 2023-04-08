#ifndef MYMENU_H
#define MYMENU_H
#include <QMenu>
//通过继承Qmenu并重写QMouseEvent()函数，实现点击后不自动关闭下拉菜单
class QMouseEvent;

class MyMenu:public QMenu
{
    Q_OBJECT
public:
    MyMenu(QWidget *parent=nullptr);
    ~MyMenu();
private:
    void mouseReleaseEvent(QMouseEvent *e);
};

#endif // MYMENU_H
