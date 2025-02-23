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
  
    $Id$
 */

#include "tdefilemetainfowidget.h"

#include <keditcl.h>
#include <tdelocale.h>
#include <knuminput.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <kstringvalidator.h>
#include <kdebug.h>

#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqspinbox.h>
#include <tqdatetimeedit.h>
#include <tqpixmap.h>
#include <tqimage.h>
#include <tqlayout.h>
#include <tqvalidator.h>

/*
  Widgets used for different types:

  bool      : QCheckBox
  int       : QSpinBox
  TQString   : KComboBox if the validator is a KStringListValidator, else lineedit
  TQDateTime : QDateTimeEdit

*/

KFileMetaInfoWidget::KFileMetaInfoWidget(KFileMetaInfoItem item,
                                         TQValidator* val,
                                         TQWidget* parent, const char* name)
    : TQWidget(parent, name),
      m_value(item.value()),
      m_item(item),
      m_validator(val)
{
    init(item, ReadWrite);
}

KFileMetaInfoWidget::KFileMetaInfoWidget(KFileMetaInfoItem item,
                                         Mode mode,
                                         TQValidator* val,
                                         TQWidget* parent, const char* name)
    : TQWidget(parent, name),
      m_value(item.value()),
      m_item(item),
      m_validator(val)
{
    init(item, mode);
}

void KFileMetaInfoWidget::init(KFileMetaInfoItem item, Mode mode)
{
    kdDebug(7033) << "*** item "  << m_item.key()
                  << " is a " << value().typeName() << endl;

    if (m_item.isEditable() && !(mode & ReadOnly))
        m_widget = makeWidget();
    else
        switch (m_value.type())
        {
            case TQVariant::Image :
                m_widget = new TQLabel(this, "info image");
                static_cast<TQLabel*>(m_widget)->setPixmap(TQPixmap(m_value.toImage()));
                break;
            case TQVariant::Pixmap :
                m_widget = new TQLabel(this, "info pixmap");
                static_cast<TQLabel*>(m_widget)->setPixmap(m_value.toPixmap());
                break;
            default:
                m_widget = new TQLabel(item.string(true), this, "info label");
        }

    (new TQHBoxLayout(this))->addWidget(m_widget);
}

KFileMetaInfoWidget::~KFileMetaInfoWidget()
{
}

TQWidget* KFileMetaInfoWidget::makeWidget()
{
    TQString valClass;
    TQWidget* w;

    switch (m_value.type())
    {
        case TQVariant::Invalid:     // no type
            // just make a label
            w = new TQLabel(i18n("<Error>"), this, "label");
            break;

        case TQVariant::Int:         // an int
        case TQVariant::UInt:        // an unsigned int
            w = makeIntWidget();
            break;

        case TQVariant::Bool:        // a bool
            w = makeBoolWidget();
            break;

        case TQVariant::Double:      // a double
            w = makeDoubleWidget();
            break;


        case TQVariant::Date:        // a QDate
            w = makeDateWidget();
            break;

        case TQVariant::Time:        // a QTime
            w = makeTimeWidget();
            break;

        case TQVariant::DateTime:    // a QDateTime
            w = makeDateTimeWidget();
            break;

#if 0
        case TQVariant::Size:        // a QSize
        case TQVariant::String:      // a TQString
        case TQVariant::List:        // a QValueList
        case TQVariant::Map:         // a QMap
        case TQVariant::StringList:  //  a QStringList
        case TQVariant::Font:        // a QFont
        case TQVariant::Pixmap:      // a QPixmap
        case TQVariant::Brush:       // a QBrush
        case TQVariant::Rect:        // a QRect
        case TQVariant::Color:       // a TQColor
        case TQVariant::Palette:     // a QPalette
        case TQVariant::ColorGroup:  // a QColorGroup
        case TQVariant::IconSet:     // a QIconSet
        case TQVariant::Point:       // a QPoint
        case TQVariant::Image:       // a TQImage
        case TQVariant::CString:     // a QCString
        case TQVariant::PointArray:  // a QPointArray
        case TQVariant::Region:      // a QRegion
        case TQVariant::Bitmap:      // a QBitmap
        case TQVariant::Cursor:      // a QCursor
        case TQVariant::ByteArray:   // a QByteArray
        case TQVariant::BitArray:    // a QBitArray
        case TQVariant::SizePolicy:  // a QSizePolicy
        case TQVariant::KeySequence: // a QKeySequence
#endif
        default:
            w = makeStringWidget();
    }

    kdDebug(7033) << "*** item " << m_item.key()
                  << "is a " << m_item.value().typeName() << endl;
    if (m_validator)
        kdDebug(7033) << " and validator is a " << m_validator->className() << endl;

    kdDebug(7033) << "*** created a " << w->className() << " for it\n";

    return w;
}

// ****************************************************************
// now the different methods to make the widgets for specific types
// ****************************************************************

TQWidget* KFileMetaInfoWidget::makeBoolWidget()
{
    TQCheckBox* cb = new TQCheckBox(this, "metainfo bool widget");
    cb->setChecked(m_item.value().toBool());
    connect(cb, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged(bool)));
    return cb;
}

TQWidget* KFileMetaInfoWidget::makeIntWidget()
{
    TQSpinBox* sb = new TQSpinBox(this, "metainfo integer widget");
    sb->setValue(m_item.value().toInt());

    if (m_validator)
    {
        if (m_validator->inherits("TQIntValidator"))
        {
            sb->setMinValue(static_cast<TQIntValidator*>(m_validator)->bottom());
            sb->setMaxValue(static_cast<TQIntValidator*>(m_validator)->top());
        }
        reparentValidator(sb, m_validator);
        sb->setValidator(m_validator);
    }

    // make sure that an uint cannot be set to a value < 0
    if (m_item.type() == TQVariant::UInt)
        sb->setMinValue(TQMAX(sb->minValue(), 0));

    connect(sb, TQ_SIGNAL(valueChanged(int)), this, TQ_SLOT(slotChanged(int)));
    return sb;
}

TQWidget* KFileMetaInfoWidget::makeDoubleWidget()
{
    KDoubleNumInput* dni = new KDoubleNumInput(m_item.value().toDouble(),
                                               this, "metainfo double widget");


    if (m_validator)
    {
        if (m_validator->inherits("QDoubleValidator"))
        {
            dni->setMinValue(static_cast<TQDoubleValidator*>(m_validator)->bottom());
            dni->setMaxValue(static_cast<TQDoubleValidator*>(m_validator)->top());
        }
        reparentValidator(dni, m_validator);
    }

    connect(dni, TQ_SIGNAL(valueChanged(double)), this, TQ_SLOT(slotChanged(double)));
    return dni;
}

TQWidget* KFileMetaInfoWidget::makeStringWidget()
{
    if (m_validator && m_validator->inherits("KStringListValidator"))
    {
        KComboBox* b = new KComboBox(true, this, "metainfo combobox");
        KStringListValidator* val = static_cast<KStringListValidator*>
                                                    (m_validator);
        b->insertStringList(val->stringList());
        b->setCurrentText(m_item.value().toString());
        connect(b, TQ_SIGNAL(activated(const TQString &)), this, TQ_SLOT(slotComboChanged(const TQString &)));
        b->setValidator(val);
        reparentValidator(b, val);
        return b;
    }

    if ( m_item.attributes() & KFileMimeTypeInfo::MultiLine ) {
        KEdit *edit = new KEdit( this );
        edit->setText( m_item.value().toString() );
        connect( edit, TQ_SIGNAL( textChanged() ),
                 this, TQ_SLOT( slotMultiLineEditChanged() ));
        // can't use a validator with a TQTextEdit, but we may need to delete it
        if ( m_validator )
            reparentValidator( edit, m_validator );
        return edit;
    }

    KLineEdit* e = new KLineEdit(m_item.value().toString(), this);
    if (m_validator)
    {
        e->setValidator(m_validator);
        reparentValidator(e, m_validator);
    }
    connect(e,    TQ_SIGNAL(textChanged(const TQString&)),
            this, TQ_SLOT(slotLineEditChanged(const TQString&)));
    return e;
}

TQWidget* KFileMetaInfoWidget::makeDateWidget()
{
  TQWidget *e = new TQDateEdit(m_item.value().toDate(), this);
  connect(e,    TQ_SIGNAL(valueChanged(const TQDate&)),
          this, TQ_SLOT(slotDateChanged(const TQDate&)));
  return e;
}

TQWidget* KFileMetaInfoWidget::makeTimeWidget()
{
  return new TQTimeEdit(m_item.value().toTime(), this);
}

TQWidget* KFileMetaInfoWidget::makeDateTimeWidget()
{
  return new TQDateTimeEdit(m_item.value().toDateTime(), this);
}

void KFileMetaInfoWidget::reparentValidator( TQWidget *widget,
                                             TQValidator *validator )
{
    if ( !validator->parent() )
        widget->insertChild( validator );
}

// ****************************************************************
// now the slots that let us get notified if the value changed in the child
// ****************************************************************

void KFileMetaInfoWidget::slotChanged(bool value)
{
    Q_ASSERT(m_widget->inherits("TQComboBox"));
    m_value = TQVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotChanged(int value)
{
    Q_ASSERT(m_widget->inherits("TQSpinBox"));
    m_value = TQVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotChanged(double value)
{
    Q_ASSERT(m_widget->inherits("KDoubleNumInput"));
    m_value = TQVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotComboChanged(const TQString &value)
{
    Q_ASSERT(m_widget->inherits("KComboBox"));
    m_value = TQVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotLineEditChanged(const TQString& value)
{
    Q_ASSERT(m_widget->inherits("KLineEdit"));
    m_value = TQVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

// that may be a little expensive for long texts, but what can we do?
void KFileMetaInfoWidget::slotMultiLineEditChanged()
{
    Q_ASSERT(m_widget->inherits("TQTextEdit"));
    m_value = TQVariant( static_cast<const TQTextEdit*>( sender() )->text() );
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotDateChanged(const TQDate& value)
{
    Q_ASSERT(m_widget->inherits("TQDateEdit"));
    m_value = TQVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotTimeChanged(const TQTime& value)
{
    Q_ASSERT(m_widget->inherits("TQTimeEdit"));
    m_value = TQVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

void KFileMetaInfoWidget::slotDateTimeChanged(const TQDateTime& value)
{
    Q_ASSERT(m_widget->inherits("TQDateTimeEdit"));
    m_value = TQVariant(value);
    emit valueChanged(m_value);
    m_dirty = true;
}

#include "tdefilemetainfowidget.moc"
