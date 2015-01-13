/*
 * Copyright (C) 2012  Dan Vratil <dvratil@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "logs-import-dialog.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QIcon>
#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QPushButton>
#include <QListWidget>

#include <KLocalizedString>

#include <KTp/logs-importer.h>
#include <TelepathyQt/Account>

Q_DECLARE_METATYPE(Tp::AccountPtr);

LogsImportDialog::LogsImportDialog(QObject *parent)
    : QDialog()
{
    Q_UNUSED(parent);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Close, this);
    m_buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("Import Logs"));

    connect(m_buttonBox, &QDialogButtonBox::clicked, this, &LogsImportDialog::slotButtonClicked);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setWindowTitle(i18n("Import Kopete Logs"));
    setWindowIcon(QIcon::fromTheme(QLatin1String("telepathy-kde")));

    QWidget *mainWidget = new QWidget(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->addWidget(mainWidget);

    QLabel *label = new QLabel(mainWidget);
    label->setText(i18n("We have found Kopete logs that seem to match some of your KDE Telepathy accounts.\n\n"
                        "Do you want to import these logs from Kopete to KDE Telepathy?"));
    label->setWordWrap(true);
    layout->addWidget(label);

    m_accountsList = new QListWidget(mainWidget);
    m_accountsList->setIconSize(QSize(24,24));
    layout->addWidget(m_accountsList, 1);

    m_progressBar = new QProgressBar(mainWidget);
    m_progressBar->setVisible(false);
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(0);
    m_progressBar->setTextVisible(false);
    layout->addWidget(m_progressBar);

    layout->addWidget(m_buttonBox);

    setMinimumWidth(300);

    m_importer = new KTp::LogsImporter(this);
    connect(m_importer, SIGNAL(logsImported()), SLOT(importFinished()));
}

LogsImportDialog::~LogsImportDialog()
{

}

void LogsImportDialog::importLogs(const QList< Tp::AccountPtr >& accounts)
{
    Q_FOREACH(const Tp::AccountPtr &account, accounts) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(account->displayName());
        item->setIcon(QIcon::fromTheme(account->iconName()));
        item->setCheckState(Qt::Checked);
        item->setData(Qt::UserRole + 1, QVariant::fromValue(account));

        m_accountsList->addItem(item);
    }

    exec();
}

void LogsImportDialog::slotButtonClicked(QAbstractButton *button)
{
    if (m_buttonBox->standardButton(button) == QDialogButtonBox::Ok) {
        m_accountsList->setEnabled(false);
        m_progressBar->setVisible(true);
        m_buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
        m_buttonBox->button(QDialogButtonBox::Close)->setDisabled(true);

        m_row = 0;
        importFinished();
        return;
    }
}

void LogsImportDialog::importFinished()
{
    if (m_row < m_accountsList->count()) {

        /* If the account is not checked, skip to next one */
        if (m_accountsList->item(m_row)->checkState() != Qt::Checked) {
            m_row++;
            importFinished();
            return;
        }

        if (m_row > 0) {
            m_accountsList->item(m_row - 1)->setCheckState(Qt::Unchecked);
        }

        Tp::AccountPtr account = m_accountsList->item(m_row)->data(Qt::UserRole + 1).value<Tp::AccountPtr>();
        m_importer->startLogImport(account);
        m_row++;
    } else {
        m_accountsList->item(m_row - 1)->setCheckState(Qt::Unchecked);
        m_progressBar->setVisible(false);
        m_buttonBox->button(QDialogButtonBox::Close)->setEnabled(true);
        m_buttonBox->button(QDialogButtonBox::Close)->setText(i18n("Done"));
    }
}
