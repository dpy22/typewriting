#include "mytextedit.h"
#include "mainwindow.h"
#include <QKeyEvent>
#include <QTextEdit>

MyTextEdit::MyTextEdit(QWidget *parent)
    :QTextEdit(parent)
{
}

void MyTextEdit::keyReleaseEvent(QKeyEvent *keyEvent)
{
    if(keyEvent->key()==Qt::Key_Backspace&&!this->document()->isEmpty())
    {
        correct++;
        emitCorrect(correct);
    }
}
