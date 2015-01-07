/*************************************************************************
 * Copyright <2007 - 2013>  <Michael Zanetti> <mzanetti@kde.org>         *
 * Copyright <2014>       <Marcin Ziemiński> <zieminn@gmail.com>         *
 *                                                                       *
 * This program is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU General Public License as        *
 * published by the Free Software Foundation; either version 2 of        *
 * the License or (at your option) version 3 or any later version        *
 * accepted by the membership of KDE e.V. (or its successor approved     *
 * by the membership of KDE e.V.), which shall act as a proxy            *
 * defined in Section 14 of version 3 of the license.                    *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

#include "authenticationwizard.h"
#include "ktp-debug.h"

#include <KTp/OTR/channel-adapter.h>
#include <KLocale>
#include <KNotification>
#include <KIconLoader>
#include <KWindowSystem>

#include <QGroupBox>
#include <QProgressBar>
namespace
{

    class WaitPage: public QWizardPage
    {
        public:

            WaitPage(const QString &text) : canContinue(false)
        {
            canContinue = false;
            setTitle(i18nc("@title","Authenticating contact..."));
            QVBoxLayout *layout = new QVBoxLayout();
            layout->addWidget(new QLabel(text));
            layout->addStretch();
            QProgressBar *progressBar = new QProgressBar();
            progressBar->setMinimum(0);
            progressBar->setMaximum(0);
            layout->addWidget(progressBar);
            layout->addStretch();
            setCommitPage(true);
            setLayout(layout);
        }

            void ready()
            {
                canContinue = true;
            }

        protected:
            virtual bool isComplete() const
            {
                return canContinue;
            }

        private:
            bool canContinue;
    };

    QList<AuthenticationWizard*> wizardList;
}

AuthenticationWizard::AuthenticationWizard(
        KTp::ChannelAdapter *chAdapter,
        const QString &contact,
        QWidget *parent,
        bool initiate,
        const QString &question)
    : QWizard(parent),
    chAdapter(chAdapter),
    contact(contact),
    question(question),
    initiate(initiate)
{

	wizardList.append(this);
	setAttribute(Qt::WA_DeleteOnClose);

	setPage(Page_SelectMethod, createIntroPage());
	setPage(Page_QuestionAnswer, createQAPage());
	setPage(Page_SharedSecret, createSSPage());
	setPage(Page_ManualVerification, createMVPage());
	setPage(Page_Wait1, new WaitPage(i18n("Waiting for <b>%1</b>...", contact)));
	setPage(Page_Wait2, new WaitPage(i18n("Checking if answers match...")));
	setPage(Page_Final, createFinalPage());

	if(!initiate) {
		if(question.isEmpty()) {
			setStartId(Page_SharedSecret);
		} else {
			setStartId(Page_QuestionAnswer);
		}
	}

	connect(this, SIGNAL(rejected()), this, SLOT(cancelVerification()));
	connect(rbQA, SIGNAL(clicked()), this, SLOT(updateInfoBox()));
	connect(rbSS, SIGNAL(clicked()), this, SLOT(updateInfoBox()));
	connect(rbMV, SIGNAL(clicked()), this, SLOT(updateInfoBox()));

	updateInfoBox();

	resize(rbMV->width() * 1.05, rbMV->width() * 0.5);
	show();
}


AuthenticationWizard::~AuthenticationWizard()
{
	wizardList.removeAll(this);
}

AuthenticationWizard *AuthenticationWizard::findWizard(KTp::ChannelAdapter *chAdapter)
{
	for(int i = 0; i < wizardList.size(); i++) {
		if(wizardList.at(i)->chAdapter == chAdapter) {
			return wizardList.at(i);
		}
	}
	return 0;
}

QWizardPage *AuthenticationWizard::createIntroPage()
{

	QWizardPage *page = new QWizardPage();
	page->setTitle(i18nc("@title", "Select authentication method"));

	rbQA = new QRadioButton(i18n("Question and Answer"));
	rbSS = new QRadioButton(i18n("Shared Secret"));
	rbMV = new QRadioButton(i18n("Manual fingerprint verification"));

	QGroupBox *frame = new QGroupBox();
	QVBoxLayout *frameLayout = new QVBoxLayout();
	frame->setLayout(frameLayout);
	infoLabel = new QLabel();
	infoLabel->setWordWrap(true);
	frameLayout->addWidget(infoLabel);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(rbQA);
	layout->addWidget(rbSS);
	layout->addWidget(rbMV);

	layout->addSpacing(30);
	layout->addWidget(frame);

	page->setLayout(layout);

	rbQA->setChecked(true);

	return page;
}

QWizardPage *AuthenticationWizard::createQAPage()
{
	QWizardPage *page = new QWizardPage();
	QGridLayout *layout = new QGridLayout();
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 0, 0);

	if(initiate) {
		page->setTitle(i18nc("@title", "Question and Answer"));

		lQuestion = new QLabel(i18nc("@info", "Enter a question that only <b>%1</b> is able to answer:",
                    contact));
		layout->addWidget(lQuestion);
		leQuestion = new QLineEdit();
		layout->addWidget(leQuestion);
		lAnswer = new QLabel(i18nc("@info", "Enter the answer to your question:"));
		layout->addWidget(lAnswer);
	} else {
		if(!question.isEmpty()) {
			page->setTitle(i18nc("@info", "Authentication with <b>%1</b>", contact));
			lQuestion = new QLabel(i18nc("@info", "<b>%1</b> would like to verify your authentication."
                        "Please answer the following question in the field below:", contact));
            layout->setRowMinimumHeight(1, 30);
			lQuestion->setWordWrap(true);
			layout->addWidget(lQuestion);
			lAnswer = new QLabel(question);
            QFont font = lAnswer->font();
            font.setItalic(true);
            lAnswer->setFont(font);
			lAnswer->setWordWrap(true);
			layout->addWidget(lAnswer);
		}
	}
	leAnswer = new QLineEdit();
	layout->addWidget(leAnswer);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 5, 0);

	page->setLayout(layout);
	page->setCommitPage(true);
	return page;
}

QWizardPage *AuthenticationWizard::createSSPage()
{
	QWizardPage *page = new QWizardPage();
	QGridLayout *layout = new QGridLayout();
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 0, 0);

	if(initiate) {
		page->setTitle(i18nc("@title", "Shared Secret"));

		layout->addWidget(new QLabel(i18nc("@info", "Enter a secret passphrase known only to you and <b>%1</b>:", contact)));
	} else {
        page->setTitle(i18nc("@title", "Authentication with <b>%1</b>", contact));
		layout->addWidget(new QLabel(i18nc("@info", "Enter the secret passphrase known only to you and <b>%1</b>:", contact)));
	}
	leSecret = new QLineEdit();
	layout->addWidget(leSecret);

    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 4, 0);
	page->setLayout(layout);
	page->setCommitPage(true);
	return page;
}

QWizardPage *AuthenticationWizard::createMVPage()
{
	QWizardPage *page = new QWizardPage();
	page->setTitle(i18nc("@title", "Manual Verification"));

	QGridLayout *layout = new QGridLayout();
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 0, 0);

	QLabel *lMessage1 = new QLabel(i18nc("@info",
                "Contact <b>%1</b> via another secure channel and verify that the following fingerprint is correct:", contact));
	lMessage1->setWordWrap(true);
	layout->addWidget(lMessage1);
    QLabel *lFingerprint = new QLabel(QLatin1String("<b>") + chAdapter->remoteFingerprint() + QLatin1String("</b>"));
    lFingerprint->setAlignment(Qt::AlignCenter);
    lFingerprint->setTextInteractionFlags(Qt::TextSelectableByMouse);
	layout->addWidget(lFingerprint);

	cbManualAuth = new QComboBox();
	cbManualAuth->addItem(i18nc("@item:inlistbox ...verified that", "I have not"));
	cbManualAuth->addItem(i18nc("@item:inlistbox ...verified that", "I have"));
	cbManualAuth->setSizeAdjustPolicy(QComboBox::AdjustToContents);

	if(chAdapter->otrTrustLevel() == KTp::OTRTrustLevelPrivate) {
		cbManualAuth->setCurrentIndex(1);
	} else {
		cbManualAuth->setCurrentIndex(0);
	}

	QLabel *lMessage2 = new QLabel(i18nc("@info:label I have...",
                "verified that this is in fact the correct fingerprint for <b>%1</b>.", contact));
	lMessage2->setWordWrap(true);

	QHBoxLayout *verifyLayout = new QHBoxLayout();
	verifyLayout->addWidget(cbManualAuth, 0, Qt::AlignLeft);
    verifyLayout->addSpacing(5);
	verifyLayout->addWidget(lMessage2, 1);

	QFrame *frame = new QFrame();
	frame->setLayout(verifyLayout);
	layout->addWidget(frame);

    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 6, 0);
    layout->setVerticalSpacing(15);
	page->setLayout(layout);

	return page;
}

QWizardPage *AuthenticationWizard::createFinalPage()
{
	QWizardPage *page = new QWizardPage();
	QGridLayout *layout = new QGridLayout();
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 0, 0);

	lFinal = new QLabel();
	lFinal->setWordWrap(true);
	layout->addWidget(lFinal);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 2, 0);

	page->setLayout(layout);

	return page;
}

int AuthenticationWizard::nextId() const
{
	if(currentId() == Page_SelectMethod) {
		if(rbQA->isChecked())
			return Page_QuestionAnswer;
		if(rbSS->isChecked())
			return Page_SharedSecret;
		if(rbMV->isChecked())
			return Page_ManualVerification;
	}
	if(currentId() == Page_SharedSecret || currentId() == Page_QuestionAnswer) {
		if(initiate) {
			return Page_Wait1;
		} else {
			return Page_Wait2;
		}
	}
	if(currentId() == Page_Wait1) {
		return Page_Wait2;
	}
	if(currentId() == Page_Wait2) {
		return Page_Final;
	}
	return -1;
}

bool AuthenticationWizard::validateCurrentPage()
{
	qCDebug(KTP_TEXTUI_LIB) << "currentId:" << currentId();
	switch(currentId()) {
		case 1:
			if(initiate) {
                chAdapter->startPeerAuthenticationQA(leQuestion->text(), leAnswer->text());
			} else {
                chAdapter->respondPeerAuthentication(leAnswer->text());
			}
			break;
		case 2:
			if(initiate) {
                chAdapter->startPeerAuthenticationSS(leSecret->text());
			} else {
                chAdapter->respondPeerAuthentication(leSecret->text());
			}
			break;
		case 3:
			if(cbManualAuth->currentIndex() == 0 ) {
                chAdapter->trustFingerprint(chAdapter->remoteFingerprint(), false);
			} else {
                chAdapter->trustFingerprint(chAdapter->remoteFingerprint(), true);
			}
			break;
	}
	return true;
}

void AuthenticationWizard::cancelVerification()
{
	qCDebug(KTP_TEXTUI_LIB) << "cancelVerification...";
	if(!initiate){
        chAdapter->abortPeerAuthentication();
	}
}

void AuthenticationWizard::nextState()
{
    qCDebug(KTP_TEXTUI_LIB);
	if(currentId() == Page_Wait1) {
		static_cast<WaitPage*>(currentPage())->ready();
		next();
	}
}

void AuthenticationWizard::finished(bool success)
{
	qCDebug(KTP_TEXTUI_LIB) << "authWizard finished";
	if(currentId() == Page_Wait2){
		qCDebug(KTP_TEXTUI_LIB) << "Yes, in wait_page2";
		static_cast<WaitPage*>(currentPage())->ready();
		next();
		if(success) {
			qCDebug(KTP_TEXTUI_LIB) << "auth succeeded";
			currentPage()->setTitle(i18n("Authentication successful"));
			if(!question.isEmpty()|| rbQA->isChecked()) {
				if(initiate){
					qCDebug(KTP_TEXTUI_LIB) << "initiate";
					lFinal->setText(i18n("The authentication with <b>%1</b> has been completed successfully."
                                " The conversation is now secure.", contact));
				} else {
					qCDebug(KTP_TEXTUI_LIB) << "not initiate";
                    lFinal->setText(i18n("<b>%1</b> has successfully authenticated you."
                                " You may want to authenticate this contact as well by asking your own question.", contact));
				}
			} else {
				lFinal->setText(i18n("The authentication with <b>%1</b> has been completed successfully. "
                            "The conversation is now secure.", contact));
			}
		} else {
			currentPage()->setTitle(i18n("Authentication failed"));
			lFinal->setText(i18n("The authentication with <b>%1</b> has failed."
                        " To make sure you are not talking to an imposter, "
                        "try again using the manual fingerprint verification method."
                        " Note that the conversation is now insecure.", contact));
		}
	}

	setOption(QWizard::NoCancelButton, true);

}

void AuthenticationWizard::aborted()
{
	if(currentId() == Page_SharedSecret || currentId() == Page_QuestionAnswer) {
		next();
	}
	if(currentId() == Page_Wait1){
		next();
	}
	if(currentId() == Page_Wait2){
		next();
	}
	currentPage()->setTitle(i18n("Authentication aborted"));
	lFinal->setText(i18n("<b>%1</b> has aborted the authentication process."
                " To make sure you are not talking to an imposter, "
                "try again using the manual fingerprint verification method.", contact));

	setOption(QWizard::NoCancelButton, true);
}

void AuthenticationWizard::updateInfoBox(){
	if(rbQA->isChecked()) {
		infoLabel->setText(i18n("Ask <b>%1</b> a question, the answer to which is known only to you and them."
                    " If the answer does not match, you may be talking to an imposter.", contact));
	} else if(rbSS->isChecked()) {
		infoLabel->setText(i18n("Pick a secret known only to you and <b>%1</b>. If the secret does not match, "
                    "you may be talking to an imposter. Do not send the secret through the chat window, "
                    "or this authentication method could be compromised with ease.", contact));
	} else {
		infoLabel->setText(i18n("Verify <b>%1's</b> fingerprint manually. "
                    "For example via a phone call or signed (and verified) email.", contact));
	}
}

void AuthenticationWizard::notificationActivated( unsigned int id)
{
	qCDebug(KTP_TEXTUI_LIB) << "notificationActivated. ButtonId" << id;
	if(id == 1) {
        this->raise();
        KWindowSystem::forceActiveWindow(this->winId());
	}
}
