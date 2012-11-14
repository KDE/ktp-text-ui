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

#include <KDialog>
#include <TelepathyQt/Types>

namespace KTp {
class LogsImporter;
}

class QProgressBar;
class KListWidget;
class LogsImportDialogPrivate;

class LogsImportDialog : public KDialog
{
    Q_OBJECT

  public:
    LogsImportDialog(QObject* parent);
    virtual ~LogsImportDialog();

    void importLogs(const QList<Tp::AccountPtr> &accounts);

  protected Q_SLOTS:
    virtual void slotButtonClicked(int button);

  private Q_SLOTS:
    void importFinished();

  private:
    KListWidget *m_accountsList;
    QProgressBar *m_progressBar;
    KTp::LogsImporter *m_importer;
    int m_row;


};

#endif // LOGSIMPORTDIALOG_H
