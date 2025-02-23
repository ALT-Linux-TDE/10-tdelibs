/*
 * configwidget.h
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#ifndef TDESPELL_CONFIGWIDGET_H
#define TDESPELL_CONFIGWIDGET_H

#include <tqwidget.h>
#include <tdelibs_export.h>

namespace KSpell2
{
    class Broker;
    class TDE_EXPORT ConfigWidget : public TQWidget
    {
        TQ_OBJECT
    public:
        ConfigWidget( Broker *broker, TQWidget *parent, const char *name =0 );
        ~ConfigWidget();

        bool backgroundCheckingButtonShown() const;

    public slots:
        void save();
        void setBackgroundCheckingButtonShown( bool );
        void slotDefault();
    protected slots:
        void slotChanged();

    private:
        void init( Broker *broker );
        void setFromGUI();
        void setCorrectLanguage( const TQStringList& langs );

    private:
        class Private;
        Private *d;
    };
}

#endif
