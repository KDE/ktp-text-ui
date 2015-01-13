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

#ifndef LOGSIMPORTDIALOG_H
#define LOGSIMPORTDIALOG_H

#include <QDialog>
#include <TelepathyQt/Types>

namespace KTp {
class LogsImporter;
}

class QAbstractButton;
class QDialogButtonBox;
class QProgressBar;
class QListWidget;
class LogsImportDialogPrivate;

class LogsImportDialog : public QDialog
{
    Q_OBJECT

  public:
    LogsImportDialog(QObject* parent);
    virtual ~LogsImportDialog();


    void importLogs(const QList<Tp::AccountPtr> &accounts);

  protected Q_SLOTS:
    virtual void slotButtonClicked(QAbstractButton *button);

  private Q_SLOTS:
    void importFinished();

  private:
    QDialogButtonBox *m_buttonBox;
    QListWidget *m_accountsList;
    QProgressBar *m_progressBar;
    KTp::LogsImporter *m_importer;
    int m_row;


};

#endif // LOGSIMPORTDIALOG_H
