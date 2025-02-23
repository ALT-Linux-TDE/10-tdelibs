/* This file is part of the KDE libraries
   Copyright (C) 2005 David Faure <faure@kde.org>

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

#ifndef PASTEDIALOG_H
#define PASTEDIALOG_H

#include <kdialogbase.h>

class TQComboBox;
class KLineEdit;
class TQLabel;

namespace TDEIO {

/**
 * @internal
 * Internal class used by paste.h. DO NOT USE.
 * @since 3.5
 */
class PasteDialog : public KDialogBase
{
    TQ_OBJECT
public:
    PasteDialog( const TQString &caption, const TQString &label,
                 const TQString &value, const TQStringList& items,
                 TQWidget *parent, bool clipboard );

    TQString lineEditText() const;
    int comboItem() const;
    bool clipboardChanged() const { return m_clipboardChanged; }

private slots:
    void slotClipboardDataChanged();

private:
    TQLabel* m_label;
    KLineEdit* m_lineEdit;
    TQComboBox* m_comboBox;
    bool m_clipboardChanged;

    class Private;
    Private* d;
};

} // namespace


#endif /* PASTEDIALOG_H */

