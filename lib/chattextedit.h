#ifndef CHATTEXTEDIT_H
#define CHATTEXTEDIT_H

#include <QTextEdit>

class ChatTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit ChatTextEdit(QWidget *parent = 0);

signals:

public slots:
    /** wraps setFontWeight to a simple on/off bold) */
    void setFontBold(bool);
};

#endif // CHATTEXTEDIT_H
