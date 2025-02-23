/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Rolf Magnus <ramagnus@kde.org>

    library is free software; you can redistribute it and/or
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

#ifndef __TDEFILEMETAINFOWIDGET_H__
#define __TDEFILEMETAINFOWIDGET_H__

#include <tqwidget.h>
#include <tqvariant.h>
#include <tdefilemetainfo.h>

/*!
 * A widget to display file meta infos (like id3 for mp3)
 */
class TDEIO_EXPORT KFileMetaInfoWidget: public TQWidget
{
    TQ_OBJECT
public:
    enum Mode
    {
      ReadOnly  = 1,  ///Only display the meta infos, and do not permit the user to edit them
      ReadWrite = 0,  ///Permits user to edit the displayed meta-info
      Reserve = 0xff
    };
  
    KFileMetaInfoWidget(KFileMetaInfoItem item, TQValidator* val = 0,
                        TQWidget* parent = 0, const char* name = 0);

    KFileMetaInfoWidget(KFileMetaInfoItem item, Mode mode, TQValidator* val = 0,
                        TQWidget* parent = 0, const char* name = 0);

    virtual ~KFileMetaInfoWidget();

    bool apply()
    {
        return m_item.isEditable() && m_item.setValue(m_value);
    }

    void setValue(const TQVariant& value)  { m_value = value;    }
    TQVariant value()const                      { return m_value;     }
    TQValidator* validator() const              { return m_validator; }
    KFileMetaInfoItem item()const              { return m_item;      }

signals:
    void valueChanged(const TQVariant& value);

protected:
    void reparentValidator(TQWidget *widget, TQValidator *validator);
    virtual TQWidget* makeWidget();

    TQWidget* makeBoolWidget();
    TQWidget* makeIntWidget();
    TQWidget* makeDoubleWidget();
    TQWidget* makeStringWidget();
    TQWidget* makeDateWidget();
    TQWidget* makeTimeWidget();
    TQWidget* makeDateTimeWidget();

private slots:
    void slotChanged(bool value);
    void slotChanged(int value);
    void slotChanged(double value);
    void slotComboChanged(const TQString &value);
    void slotLineEditChanged(const TQString& value);
    void slotMultiLineEditChanged();
    void slotDateChanged(const TQDate& value);
    void slotTimeChanged(const TQTime& value);
    void slotDateTimeChanged(const TQDateTime& value);

private:
    void init(KFileMetaInfoItem item, Mode mode);

    TQVariant          m_value;  // the value will be saved here until apply() is called
    KFileMetaInfoItem m_item;
    TQWidget*          m_widget;
    TQValidator*       m_validator;
    bool              m_dirty : 1;
};

#endif
