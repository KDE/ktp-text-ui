#ifndef NOTIFYFILTER_H
#define NOTIFYFILTER_H

#include "chat-widget.h"

#include <KTp/abstract-message-filter.h>

class NotifyFilter : public KTp::AbstractMessageFilter
{
    Q_OBJECT
public:
    explicit NotifyFilter(ChatWidget *widget);
    void filterMessage(KTp::Message &message, const KTp::MessageContext &context);

private:
    ChatWidget *m_widget;
};

#endif // NOTIFYFILTER_H
