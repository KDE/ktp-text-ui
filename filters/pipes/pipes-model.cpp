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

#include "pipes-model.h"
#include <KDebug>

PipesModel::PipesModel() : m_pipes(m_prefs.pipeList())
{
    kDebug();
    m_columnNames << i18n("Command") << i18n("Direction") << i18n("Format");
}

PipesModel::~PipesModel()
{
    kDebug();
}

QVariant PipesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return m_columnNames[section];
    }

    return QVariant();
}

QVariant PipesModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole && index.isValid()) {
        PipesPrefs::Pipe pipe = m_pipes.at(index.row());
        switch (index.column()) {
            case DirectionColumn :
                return pipe.direction;
            case ExecutableColumn :
                return pipe.executable;
            case FormatColumn :
                return pipe.format;
        }
    }

    return QVariant();
}

bool PipesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    kDebug() << "setting" << index << "to" << value;

    PipesPrefs::Pipe pipe = m_pipes.at(index.row());
    switch (index.column()) {
        case DirectionColumn :
            pipe.direction = (PipesPrefs::PipeDirection) value.toInt();
            break;
        case FormatColumn :
            pipe.format = (PipesPrefs::MessageFormat) value.toInt();
            break;
        case ExecutableColumn :
            pipe.executable = value.toString();
            break;
        default:
            return false; //shouldn't really happen
    }

    m_pipes[index.row()] = pipe;
    Q_EMIT dataChanged(index, index);

    return true;
}

int PipesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED (parent);
    return m_columnNames.length();
}

int PipesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED (parent);
    return m_pipes.size();
}

Qt::ItemFlags PipesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    return flags;
}

void PipesModel::clear()
{
    beginResetModel();
    m_pipes = PipesPrefs::PipeList();
    endResetModel();
}

void PipesModel::revert()
{
    beginResetModel();
    m_prefs.load();
    m_pipes = m_prefs.pipeList();
    endResetModel();
}

bool PipesModel::submit()
{
    Q_FOREACH (PipesPrefs::Pipe pipe, m_pipes) {
        kDebug() << pipe;
    }
    m_prefs.setPipeList(m_pipes);
    m_prefs.save();
    return true;
}

bool PipesModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_ASSERT (count == 1);
    Q_UNUSED (parent)

    beginInsertRows(parent, row, row);
    m_pipes.insert(row, PipesPrefs::Pipe());
    endInsertRows();

    return true;
}

bool PipesModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_ASSERT (count == 1);
    Q_UNUSED (parent)

    beginRemoveRows(parent, row, row);
    m_pipes.removeAt(row);
    endRemoveRows();

    return false;
}
