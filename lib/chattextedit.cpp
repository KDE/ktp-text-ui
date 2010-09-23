#include "chattextedit.h"
#include <QMenu>
#include <QContextMenuEvent>
#include <QAction>

class ChatTextEditPrivate
{
    QWidget* formatToolbar;
};

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

void ChatTextEdit::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    menu->addActions(actions());
    menu->exec(event->globalPos());
    delete menu;
}
