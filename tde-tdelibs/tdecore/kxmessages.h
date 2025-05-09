/****************************************************************************

 Copyright (C) 2001-2003 Lubos Lunak        <l.lunak@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

****************************************************************************/

#ifndef __KXMESSAGES_H
#define __KXMESSAGES_H

#include <tqwidget.h>
#include <tqcstring.h>
#include <tqmap.h>
#include <tdelibs_export.h>

#ifdef TQ_WS_X11
#include <X11/X.h>

class TQString;

class KXMessagesPrivate;
/**
 * Sending string messages to other applications using the X Client Messages.
 *
 * Used internally by TDEStartupInfo. You usually don't want to use this, use DCOP
 * instead.
 *
 * @author Lubos Lunak <l.lunak@kde.org>
 */
// KDE4 - make this internal for TDEStartupInfo only?
class TDECORE_EXPORT KXMessages
    : public TQWidget
    {
    TQ_OBJECT
    public:
	/**
	 * Creates an instance which will receive X messages.
	 *
	 * @param accept_broadcast if non-NULL, all broadcast messages with 
	 *                         this message type will be received.
	 * @param parent the parent of this widget
         * @param obsolete always set to false (needed for backwards compatibility
         *                 with KDE3.1 and older)
	 */
        KXMessages( const char* accept_broadcast, TQWidget* parent, bool obsolete );
        /**
         * @deprecated
         * This method is equivalent to the other constructor with obsolete = true.
         */
        KXMessages( const char* accept_broadcast = NULL, TQWidget* parent = NULL );

        virtual ~KXMessages();
	/**
	 * Sends the given message with the given message type only to given
         * window.
         * 
         * @param w X11 handle for the destination window
	 * @param msg_type the type of the message
	 * @param message the message itself
         * @param obsolete always set to false (needed for backwards compatibility
         *                 with KDE3.1 and older)
	 */
        void sendMessage( WId w, const char* msg_type, const TQString& message,
            bool obsolete );
	/**
         * @deprecated
         * This method is equivalent to sendMessage() with obsolete = true.
	 */
        void sendMessage( WId w, const char* msg_type, const TQString& message );
	/**
	 * Broadcasts the given message with the given message type.
	 * @param msg_type the type of the message
	 * @param message the message itself
         * @param screen X11 screen to use, -1 for the default
         * @param obsolete always set to false (needed for backwards compatibility
         *                 with KDE3.1 and older)
	 */
        void broadcastMessage( const char* msg_type, const TQString& message,
            int screen, bool obsolete );
	/**
         * @deprecated
         * This method is equivalent to broadcastMessage() with obsolete = true.
	 */
        void broadcastMessage( const char* msg_type, const TQString& message );

	/**
	 * Sends the given message with the given message type only to given
         * window.
         * 
	 * @param disp X11 connection which will be used instead of 
	 *             qt_x11display()
         * @param w X11 handle for the destination window
	 * @param msg_type the type of the message
	 * @param message the message itself
         * @param obsolete always set to false (needed for backwards compatibility
         *                 with KDE3.1 and older)
	 * @return false when an error occurred, true otherwise
	 */
        static bool sendMessageX( Display* disp, WId w, const char* msg_type,
            const TQString& message, bool obsolete );
	/**
         * @deprecated
         * This method is equivalent to sendMessageX() with obsolete = true.
	 */
        static bool sendMessageX( Display* disp, WId w, const char* msg_type,
            const TQString& message );

	/**
	 * Broadcasts the given message with the given message type.
	 *
	 * @param disp X11 connection which will be used instead of 
	 *             qt_x11display()
	 * @param msg_type the type of the message
	 * @param message the message itself
         * @param screen X11 screen to use, -1 for the default
         * @param obsolete always set to false (needed for backwards compatibility
         *                 with KDE3.1 and older)
	 * @return false when an error occurred, true otherwise
	 */
        static bool broadcastMessageX( Display* disp, const char* msg_type,
            const TQString& message, int screen, bool obsolete );
	/**
         * @deprecated
         * This method is equivalent to broadcastMessageX() with obsolete = true.
	 */
        static bool broadcastMessageX( Display* disp, const char* msg_type,
            const TQString& message );
    signals:
	/**
	 * Emitted when a message was received.
	 * @param message the message that has been received
	 */
        void gotMessage( const TQString& message );
    protected:
	/**
	 * @internal
	 */
        virtual bool x11Event( XEvent* ev );
    private:
        static void send_message_internal( WId w_P, const TQString& msg_P, long mask_P,
            Display* disp, Atom atom1_P, Atom atom2_P, Window handle_P );
        TQWidget* handle;
        Atom accept_atom2;
        TQCString cached_atom_name_; // KDE4 unused
        Atom accept_atom1;
        TQMap< WId, TQCString > incoming_messages;
        KXMessagesPrivate* d;
    };

#endif
#endif
