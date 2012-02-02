
#include "message-processor.h"

class UrlFilter : public AbstractMessageFilter
{
    virtual void filterMessage(Message& message);
};

class ImageFilter : public AbstractMessageFilter
{
    virtual void filterMessage(Message& message);
};

class EmoticonFilter : public AbstractMessageFilter
{
    virtual void filterMessage(Message& message);
};