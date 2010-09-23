#include "chattextedit.h"

ChatTextEdit::ChatTextEdit(QWidget *parent) :
    QTextEdit(parent)
{
}

void ChatTextEdit::setFontBold(bool isBold)
{
    if (isBold)
    {
        setFontWeight(QFont::Bold);
    }
    else
    {
        setFontWeight(QFont::Normal);
    }
}
