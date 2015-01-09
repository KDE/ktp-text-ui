/*************************************************************************
 * Copyright <2007 - 2013>  <Michael Zanetti> <mzanetti@kde.org>         *
 * Copyright <2014>  <Marcin Ziemiński> <zieminn@gmail.com>              *
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

#ifndef AUTHENTICATIONWIZARD_HEADER
#define AUTHENTICATIONWIZARD_HEADER

#include <QWizard>

namespace KTp
{
    class ChannelAdapter;
}

class QComboBox;
class QLabel;
class QLineEdit;
class QRadioButton;

class AuthenticationWizard: public QWizard
{
	Q_OBJECT
public:
	explicit AuthenticationWizard(
            KTp::ChannelAdapter *chAdapter,
            const QString &contact,
            QWidget *parent = 0,
            bool initiate = true,
            const QString &question = QLatin1String(""));

	~AuthenticationWizard();

	static AuthenticationWizard *findWizard(KTp::ChannelAdapter *chAdapter);
	void nextState();
	void finished(bool success);
	void aborted();

protected:

	virtual int nextId() const;
	virtual bool validateCurrentPage();

private:
	enum {
        Page_SelectMethod,
        Page_QuestionAnswer,
        Page_SharedSecret,
        Page_ManualVerification,
        Page_Wait1,
        Page_Wait2,
        Page_Final
    };

    KTp::ChannelAdapter *chAdapter;
    const QString contact;

	QString question;
	bool initiate;

	QLabel *lQuestion;
	QLabel *lAnswer;
	QLabel *lSecret;
	QLabel *infoLabel;
	QLabel *lFinal;

	QLineEdit *leQuestion;
	QLineEdit *leAnswer;
	QLineEdit *leSecret;

	QRadioButton *rbQA;
	QRadioButton *rbSS;
	QRadioButton *rbMV;

	QComboBox *cbManualAuth;

	QWizardPage *createIntroPage();
	QWizardPage *createQAPage();
	QWizardPage *createSSPage();
	QWizardPage *createMVPage();
	QWizardPage *createFinalPage();

private Q_SLOTS:
	void cancelVerification();
	void updateInfoBox();
	void notificationActivated(unsigned int);
};


#endif
