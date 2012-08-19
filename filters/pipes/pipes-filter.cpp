/*
 *    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 2.1 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "pipes-filter.h"
#include "pipes-prefs.h"

#include <KPluginFactory>
#include <KDebug>
#include <KUrl>
#include <KUriFilter>
#include <KProcess>

class PipesFilter::Private
{
public:
    PipesPrefs prefs;
};

PipesFilter::PipesFilter(QObject *parent, const QVariantList &) :
    AbstractMessageFilter(parent), d(new Private)
{
}

PipesFilter::~PipesFilter()
{
    delete d;
}

void PipesFilter::filterMessage(Message &message)
{
    Q_FOREACH (PipesPrefs::Pipe pipe, d->prefs.pipeList()) {
        kDebug() << "processing " << pipe << "for direction" << message.direction() ;
        if (pipe.direction & message.direction()) {
            Q_ASSERT (pipe.format == PipesPrefs::FormatPlainText);

            QProcess process;

            kDebug() << "running program : " << pipe.executable;
            process.start(pipe.executable);

            if (!process.waitForStarted()) {
                kError() << "Could not start " << pipe.executable << ":" << process.error();
                continue;
            }
            if (process.write(message.mainMessagePart().toLatin1()) == -1) {
                kError() << "Could not write " << message.mainMessagePart() << ":" << process.errorString();
                continue;
            }
            process.closeWriteChannel();
            process.waitForFinished();
            if (process.exitCode()) {
                kError() << pipe.executable << "exited with error code" << process.exitCode();
                continue;
            }
            QByteArray buf = process.readAllStandardOutput();
//             if (buf.length() > 0) {
                message.setMainMessagePart(QLatin1String(buf));
//             }
        }
    }
}

K_PLUGIN_FACTORY(MessageFilterFactory, registerPlugin<PipesFilter>();)
K_EXPORT_PLUGIN(MessageFilterFactory("ktptextui_message_filter_pipes"))
