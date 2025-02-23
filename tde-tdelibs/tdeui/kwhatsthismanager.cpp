/*  This file is part of the KDE Libraries
 *  Copyright (C) 2004 Peter Rockai (mornfall) <mornfall@danill.sk>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */
#include "kwhatsthismanager_p.h"
#include "tqwhatsthis.h"
#include <tqvariant.h>
#include <kdebug.h>
#include <tqtextedit.h>
#include <tdelocale.h>
#include <tdeapplication.h>

KWhatsThisManager *KWhatsThisManager::s_instance = 0;

class KWhatsThisUndefined : public TQWhatsThis
{
    public:
        KWhatsThisUndefined (TQWidget *);
        TQString text (const TQPoint &);
    public slots:
        bool clicked (const TQString &);
    protected:
        TQWidget *m_widget;
};

KWhatsThisUndefined::KWhatsThisUndefined (TQWidget *w)
    : TQWhatsThis (w)
{
    m_widget = w;
}

TQString KWhatsThisUndefined::text (const TQPoint &)
{
    if (!m_widget)
        return "";
    TQString txt = i18n ("<b>Not Defined</b><br>There is no \"What's This?\""
            " help assigned to this widget. If you want to help us to "
            " describe the widget, you are welcome to <a href=\"submit"
            "-whatsthis\">send us your own \"What's This?\" help</a> for it.");
    TQString parent;
    if (m_widget -> parentWidget ())
        parent = TQWhatsThis::textFor (m_widget -> parentWidget ());
        if (parent != txt)
            if (! parent . isEmpty ())
                return parent;
    return txt;
}

bool KWhatsThisUndefined::clicked (const TQString& href)
{
    if (href == "submit-whatsthis") {
        TQWidget *w = m_widget;
        TQString body;
        body . append ("Widget text: '" + (m_widget -> property ("text") . toString ()) + "'\n");
        TQString dsc = TQString ("current --> ") + m_widget -> name ();
        dsc . append (TQString (" (") + m_widget -> className () + ")\n");
        for (w = m_widget; w && w != m_widget -> topLevelWidget (); w = w -> parentWidget ()) {
            dsc . append (w -> name ());
            dsc . append (TQString (" (") + w -> className () + ")\n");
        }
        w = m_widget -> topLevelWidget ();
        if (w) {
            dsc . append ("toplevel --> ");
            dsc . append (w -> name ());
            dsc . append (TQString (" (") + w -> className () + ")\n");
        }
        body . append (dsc);
        TQString subj ("What's This submission: ");
        subj . append (tqApp -> argv () [0]);
        body . append ("\nPlease type in your what's this help between these lines: "
                "\n--%-----------------------------------------------------------------------\n"
                "\n--%-----------------------------------------------------------------------");
        kapp -> invokeMailer ("quality-whatsthis@kde.org", "", "", subj, body);
    }
    return TRUE;
}

void KWhatsThisManager::init ()
{
    if (s_instance)
        return;
    s_instance = new KWhatsThisManager;
}

KWhatsThisManager::KWhatsThisManager ()
{
    // go away...
    // tqApp -> installEventFilter (this);
}

bool KWhatsThisManager::eventFilter (TQObject * /*o*/, TQEvent *e)
{
    if (e -> type () == TQEvent::ChildInserted) {
        TQChildEvent *ce = (TQChildEvent *)e;
        // kdDebug () << "new qobject:" << ce -> child () << endl;
        if (ce -> child () -> isWidgetType ()) {
            TQWidget *w = (TQWidget *) (ce -> child ());
            // kdDebug () << "new qwidget:" << w << endl;
            if (TQWhatsThis::textFor (w) . isEmpty ())
                new KWhatsThisUndefined (w);
        }
    }
    return false;
}

#include "kwhatsthismanager_p.moc"

