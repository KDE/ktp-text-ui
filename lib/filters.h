
#include "message-processor.h"

#include <KEmoticons>

class UrlFilter : public AbstractMessageFilter
{
    void filterMessage(Message& message);
};

class ImageFilter : public AbstractMessageFilter
{
    void filterMessage(Message& message);
};

class EmoticonFilter : public AbstractMessageFilter
{
    void filterMessage(Message& message);
private:
    KEmoticons m_emoticons;
};