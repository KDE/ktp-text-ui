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

#ifndef PIPES_PREFS_H
#define PIPES_PREFS_H
#include <message.h>
#include <KConfigGroup>

#include <QVariant>

class PipesPrefsTest;
class PipesPrefs
{
public:
    enum PipeDirection {
        Incoming = Message::Incoming,
        Outgoing = Message::Outgoing,
        Both = Incoming | Outgoing
    };

    enum MessageFormat {
        FormatPlainText
    };

    class Pipe {
    public:
        Pipe() {}
        Pipe(const QString &executable, PipeDirection direction, MessageFormat format = FormatPlainText) : executable(executable), direction(direction), format(format) {}

        bool operator==(const Pipe &other) {
            return other.executable == executable && other.direction == direction && other.format == format;
        }

        QString executable;
        PipeDirection direction;
        MessageFormat format;
    };

    typedef QList<Pipe> PipeList;

    PipesPrefs();

    void load();
    void save();

    PipeList pipeList() const;
    void reset();

    friend class PipesPrefsTest;

protected:
    KConfigGroup config() const;

private:
    PipeList m_pipeList;
};

// Q_DECLARE_METATYPE(PipesPrefs::Pipe);
// Q_DECLARE_METATYPE(PipesPrefs::PipeList);

#endif // PIPES_PREFS_H
