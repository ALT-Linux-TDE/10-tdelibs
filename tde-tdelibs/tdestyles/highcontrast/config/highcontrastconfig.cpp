/*
Copyright (c) 2005 Luciano Montanaro <mikelima@cirulla.net>

based on the Keramick configuration dialog 
Copyright (c) 2003 Maksim Orlovich <maksim.orlovich@kdemail.net>

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

*/

#include <tqcheckbox.h>
#include <tqlayout.h>
#include <tqsettings.h>
#include <kdialog.h>
#include <tdeglobal.h>
#include <tdelocale.h>

#include "highcontrastconfig.h"

extern "C" TDE_EXPORT TQWidget* 
allocate_tdestyle_config(TQWidget* parent)
{
    return new HighContrastStyleConfig(parent);
}

HighContrastStyleConfig::HighContrastStyleConfig(
        TQWidget* parent): TQWidget(parent)
{
    // Should have no margins here, the dialog provides them
    TQVBoxLayout* layout = new TQVBoxLayout(this, 0, 0);
    TDEGlobal::locale()->insertCatalogue("tdestyle_highcontrast_config");

    wideLinesBox = new TQCheckBox(i18n("Use wider lines"), this);

    layout->add(wideLinesBox);
    layout->addStretch(1);

    TQSettings s;

    originalWideLinesState = s.readBoolEntry(
            "/highcontraststyle/Settings/wideLines", false);
    wideLinesBox->setChecked(originalWideLinesState);

    connect(wideLinesBox, TQ_SIGNAL(toggled(bool)), TQ_SLOT(updateChanged()));
}

HighContrastStyleConfig::~HighContrastStyleConfig()
{
    TDEGlobal::locale()->removeCatalogue("tdestyle_keramik_config");
}


void 
HighContrastStyleConfig::save()
{
    TQSettings s;
    s.writeEntry("/highcontraststyle/Settings/wideLines", 
            wideLinesBox->isChecked());
}

void 
HighContrastStyleConfig::defaults()
{
    wideLinesBox->setChecked(false);
    // updateChanged would be done by setChecked already
}

void 
HighContrastStyleConfig::updateChanged()
{
    if ((wideLinesBox->isChecked() == originalWideLinesState)) {
        emit changed(false);
    } else {
        emit changed(true);
    }
}

#include "highcontrastconfig.moc"
