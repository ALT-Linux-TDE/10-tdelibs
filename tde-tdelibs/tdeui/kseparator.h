/*
 *   Copyright (C) 1997  Michael Roth <mroth@wirlweb.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef __KSEPARATOR_H__
#define __KSEPARATOR_H__

#include <tqframe.h>

#include <tdelibs_export.h>

/**
 * Standard horizontal or vertical separator.
 *
 * @author Michael Roth <mroth@wirlweb.de>
 * @version $Id$
*/
class TDEUI_EXPORT KSeparator : public TQFrame
{
  TQ_OBJECT
  TQ_PROPERTY( int orientation READ orientation WRITE setOrientation )
 public:
  /**
   * Constructor.
   * @param parent parent object.
   * @param name name of the new object.
   * @param f extra TQWidget flags.
   **/
  KSeparator(TQWidget* parent=0, const char* name=0, WFlags f=0);

  /**
   * Constructor.
   * @param orientation Set the orientation of the separator.
   * Possible values are HLine or Horizontal and VLine or Vertical.
   * @param parent parent object.
   * @param name name of the new object.
   * @param f extra TQWidget flags.
   **/
  KSeparator(int orientation, TQWidget* parent=0, const char* name=0, 
	     WFlags f=0);
  
  /**
   * Returns the orientation of the separator.
   * @return int Possible values are VLine and HLine.
   **/
  int orientation() const;
  
  /**
   * Set the orientation of the separator to @p orient
   *
   * @param orient Possible values are VLine and HLine.
   */
  void setOrientation(int orient);
  
  /**
   * The recommended height (width) for a horizontal (vertical) separator.
   **/
  virtual TQSize sizeHint() const;

protected:
  /**
   * @param p pointer to painter
   */
  virtual void drawFrame( TQPainter *p );
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KSeparatorPrivate* d;
};


#endif // __KSEPARATOR_H__
