/* This file is part of the KDE libraries
   Copyright (C) 2001, 2002 Frerich Raabe <raabe@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __karrowbutton_h__
#define __karrowbutton_h__

#include <tqpushbutton.h>

#include <tdelibs_export.h>

class KArrowButtonPrivate;
/**
 * @short Draws a button with an arrow.
 *
 * Draws a button which shows an arrow pointing into a certain direction. The
 * arrow's alignment on the button depends on the direction it's pointing to,
 * e.g. a left arrow is aligned at the left border, a upwards arrow at the top
 * border. This class honors the currently configured TDEStyle when drawing
 * the arrow.
 *
 * @author Frerich Raabe
 */
class TDEUI_EXPORT KArrowButton : public TQPushButton
{
	TQ_OBJECT
	TQ_PROPERTY( int arrowType READ arrowTp WRITE setArrowTp )

	public:
		/**
		 * Constructs an arrow button.
		 *
		 * @param parent This button's parent
		 * @param arrow The direction the arrrow should be pointing in
		 * @param name An internal name for this widget
		 */
		KArrowButton(TQWidget *parent = 0, TQt::ArrowType arrow = TQt::UpArrow,
			const char *name = 0);

		/**
		 * Destructor.
		 */
		virtual ~KArrowButton();

		/**
		 * Reimplemented from TQPushButton.
		 */
		virtual TQSize sizeHint() const;

		/**
		* Returns the arrow type
		* @since 3.4
		*/
		TQt::ArrowType arrowType() const;

		// hacks for moc braindamages with enums
		int arrowTp() const { return (int) arrowType(); }
		void setArrowTp( int tp ) { setArrowType( (TQt::ArrowType) tp ); }
	public slots:
		/**
		 * Defines in what direction the arrow is pointing to. Will repaint the
		 * button if necessary.
		 *
		 * @param a The direction this arrow should be pointing in
		 */
		void setArrowType(TQt::ArrowType a);

	protected:
		/**
		 * Reimplemented from TQPushButton.
		 */
		virtual void drawButton(TQPainter *);

	protected:
		virtual void virtual_hook( int id, void* data );
	private:
		KArrowButtonPrivate *d;
};

#endif // __karrowbutton_h__
