
#include "message-processor.h"

class UrlFilter : public AbstractMessageFilter
{
    virtual void filterMessage(Message& message);
};

