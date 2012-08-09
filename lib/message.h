/*
    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef MESSAGE_H
#define MESSAGE_H

#include <TelepathyQt/Message>

#include <ktpchat_export.h>

/*!
 * \par
 * An encapsualtion of a Tp::Message that can be procesesd
 * by many MessageFilters concurrently.
 *
 * \par
 * Contains multiple parts created by plugins, to be displayed by user
 * interfaces. Also contains internal metadata for use by other plugins.
 *
 * \note
 * Methods in this class are currently *not* thread safe. They will be in a
 * later version. Setting properties concurrently is undefined. 
 *
 * \author Lasath Fernando <kde@lasath.org>
 */
class KDE_TELEPATHY_CHAT_EXPORT Message {

public:
    Message(const Tp::Message& original);

    /*! \brief The body of the message
     * \return the contents of the body of the message, as HTML
     */
    QString mainMessagePart() const;

    /*! \brief Edit the main component of the message
     *
     * \param message the string to replace the body with. Must be correct HTML
     */
    void setMainMessagePart(const QString& message);

    /*! \brief Add new visual content to the end of the message
     *
     * \par
     * Each plugin that adds visual components should call this once thier
     * processing is complete. Once a message part is added, it cannot be
     * changed!
     * 
     * \param part the content to be added, in valid HTML
     */
    void appendMessagePart(const QString &part);

    /*! \brief Construct the final procesesd content
     * \par
     * This will concatenate all the visual 'parts' of the message into one
     * (Qt supported) HTML string.
     *
     * \note
     * All user interfaces need only care about this
     *
     */
    QString finalizedMessage() const;

    /*! \brief Sets the contents of a property
     * \par
     * These messages contain meta-data for plugins in the form of 'properties'.
     * A property can be set to any QMetaType (i.e type that can stuck in a
     * QVariant) and is identified by a string (name).
     *
     * \par
     * These are set by plugins for use in other plugins, creating implicit
     * dependencies between plugins. Since these plugins are (or will be)
     * run concurrently, calling this method on a property that hasn't been set
     * yet will block until it has been set by some plugin. If it isn't set when
     * all plugins are finished, this plugin will be cancelled.
     *
     * \param name the identifier of the property
     */
    QVariant property(const char *name) const;
    void setProperty(const char *name, const QVariant &value);

    /*! \return the time the message was sent*/
    QDateTime time() const;
    /*! \return the unique token from the message*/
    QString token() const;
    /*! \return the type of the message*/
    Tp::ChannelTextMessageType type() const;

private:
    QDateTime   m_sentTime;
    QString     m_token;
    Tp::ChannelTextMessageType m_messageType;
    QVariantMap m_properties;
    QString     m_mainPart;
    QStringList m_parts;
};


#endif // MESSAGE_H
