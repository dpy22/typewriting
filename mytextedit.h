#ifndef MYTEXTEDIT_H
#define MYTEXTEDIT_H

#include <QTextEdit>
#include <QMainWindow>

class MyTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    MyTextEdit(QWidget *parent=nullptr);
private:
    void keyReleaseEvent(QKeyEvent *keyEvent);
    void emitCorrect(int a)
    {
        emit emit_correct(a);
    }

    int correct=0;
signals:
    void emit_correct(int a);//将correct发射到主界面
};

#endif // MYTEXTEDIT_H
