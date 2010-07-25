#include "chatwindow.h"
#include "ui_chatwindow.h"
#include "telepathychatmessageinfo.h"
#include "telepathychatinfo.h"


ChatWindow::ChatWindow(ChatConnection* chat, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ChatWindow),
        m_chatConnection(chat)
{
    ui->setupUi(this);
    //  //updateEnabledState(m_clientHandler->isChannelReady());
    updateEnabledState(false);

    //chat connection lifespan should be same as the chatwindow
    m_chatConnection->setParent(this);

    connect(m_chatConnection, SIGNAL(channelReadyStateChanged(bool)), SLOT(updateEnabledState(bool)));
    connect(m_chatConnection->channel().data(), SIGNAL(messageReceived(Tp::ReceivedMessage)), SLOT(handleIncomingMessage(Tp::ReceivedMessage)));
    connect(m_chatConnection->channel().data(), SIGNAL(messageSent(Tp::Message, Tp::MessageSendingFlags, QString)), SLOT(handleMessageSent(Tp::Message, Tp::MessageSendingFlags, QString)));
    connect(m_chatConnection->channel().data(), SIGNAL(chatStateChanged(Tp::ContactPtr, ChannelChatState)), SLOT(updateChatStatus(Tp::ContactPtr, ChannelChatState)));
    connect(ui->sendMessageButton, SIGNAL(released()), SLOT(sendMessage()));

    TelepathyChatInfo info;
    info.setChatName("cheese");

    ui->chatArea->initialise(info);
}

ChatWindow::~ChatWindow()
{
    delete ui;
}

void ChatWindow::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}



void ChatWindow::handleIncomingMessage(const Tp::ReceivedMessage &message)
{
    TelepathyChatMessageInfo messageInfo;
    messageInfo.setMessage(message.text());
    //messageInfo.setSenderScreenName(message.sender()->id());
    messageInfo.setTime(message.received());
    messageInfo.setMessageDirection("rtl");
    messageInfo.setSenderDisplayName(message.sender()->alias());

    ui->chatArea->addMessage(messageInfo);

    m_chatConnection->channel()->acknowledge(QList<Tp::ReceivedMessage>() << message);
}

void ChatWindow::handleMessageSent(const Tp::Message &message, Tp::MessageSendingFlags, const QString&) /*Not sure what these other args are for*/
{
    TelepathyChatMessageInfo messageInfo;
    messageInfo.setMessage(message.text());
    messageInfo.setTime(message.sent());
    messageInfo.setMessageDirection("ltr");
//    messageInfo.setSenderDisplayName(m_chatConnection->connection()->selfContact()->alias()); // selfConect() can return 0 watch out for that.

    ui->chatArea->addMessage(messageInfo);
}

void ChatWindow::sendMessage()
{
    m_chatConnection->channel()->send(ui->sendMessageBox->toPlainText());
    ui->sendMessageBox->clear();
}

void ChatWindow::updateChatStatus(Tp::ContactPtr contact, ChannelChatState state)
{
    qDebug() << contact->alias() << state;
}

void ChatWindow::updateEnabledState(bool enable)
{
    qDebug() << "setting buttons to " << enable;
    ui->sendMessageBox->setEnabled(enable);
    ui->sendMessageButton->setEnabled(enable);
}
