#ifndef CHATTEXTEDIT_H
#define CHATTEXTEDIT_H

#include <QTextEdit>

class ChatTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit ChatTextEdit(QWidget *parent = 0);

    // reimplemented
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    // reimplemented
    void resizeEvent(QResizeEvent*);
    void contextMenuEvent(QContextMenuEvent *);

private slots:
    void recalculateSize();
    void updateScrollBar();

public slots:
    /** wraps setFontWeight to a simple on/off bold) */
    void setFontBold(bool);
};

#endif // CHATTEXTEDIT_H
