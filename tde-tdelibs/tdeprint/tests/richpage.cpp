#include "richpage.h"

#include <tqlabel.h>
#include <tqspinbox.h>
#include <tqcombobox.h>
#include <tqfontdatabase.h>
#include <tqlayout.h>

RichPage::RichPage(TQWidget *parent, const char *name)
: KPrintDialogPage(parent,name)
{
	m_title = "Rich Text Options";

	margin_ = new TQSpinBox(this);
	margin_->setRange(1,999);
	margin_->setValue(72);

	fontsize_ = new TQSpinBox(this);
	fontsize_->setRange(4,100);
	fontsize_->setValue(10);

	fontname_ = new TQComboBox(this);
	QFontDatabase	db;
	QStringList	fonts = db.families();
	fontname_->insertStringList(fonts);
	fontname_->setCurrentItem(fonts.findIndex(TQString::fromLatin1("times")));
	if (fontname_->currentItem() < 0) fontname_->setCurrentItem(0);

	QLabel	*l1 = new TQLabel("Margin:",this);
	QLabel	*l2 = new TQLabel("Font name:",this);
	QLabel	*l3 = new TQLabel("Font size:",this);

	QHBoxLayout	*s1 = new TQHBoxLayout(0, 0, 10);
	QHBoxLayout	*s2 = new TQHBoxLayout(0, 0, 10);
	QVBoxLayout	*main_ = new TQVBoxLayout(this, 10, 10);

	main_->addLayout(s1,0);
	main_->addSpacing(20);
	main_->addLayout(s2,0);
	main_->addStretch(1);

	s1->addWidget(l1,0);
	s1->addWidget(margin_,0);
	s1->addStretch(1);

	s2->addWidget(l2,0);
	s2->addWidget(fontname_,0);
	s2->addSpacing(20);
	s2->addWidget(l3,0);
	s2->addWidget(fontsize_,0);
	s2->addStretch(1);
}

RichPage::~RichPage()
{
}

void RichPage::setOptions(const TQMap<TQString,TQString>& opts)
{
	TQString	value;

	value = opts["app-rich-margin"];
	if (!value.isEmpty())
		margin_->setValue(value.toInt());

	value = opts["app-rich-fontname"];
	if (!value.isEmpty())
		for (int i=0;i<fontname_->count();i++)
			if (fontname_->text(i) == value)
			{
				fontname_->setCurrentItem(i);
				break;
			}

	value = opts["app-rich-fontsize"];
	if (!value.isEmpty())
		fontsize_->setValue(value.toInt());
}

void RichPage::getOptions(TQMap<TQString,TQString>& opts, bool)
{
	opts["app-rich-margin"] = margin_->text();
	opts["app-rich-fontname"] = fontname_->currentText();
	opts["app-rich-fontsize"] = fontsize_->text();
}
