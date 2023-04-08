#include "mymenu.h"
#include "mainwindow.h"
#include <QMouseEvent>

MyMenu::MyMenu(QWidget *parent)
    : QMenu(parent)
{

}

MyMenu::~MyMenu()
{

}

void MyMenu::mouseReleaseEvent(QMouseEvent *e)
{
    QAction *action = this->actionAt(e->pos());
    if(action)
    {
        action->activate(QAction::Trigger);
    }
    else
    {
        QMenu::mouseReleaseEvent(e);
    }
}
