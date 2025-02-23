#ifndef RICHPAGE_H
#define RICHPAGE_H

#include <kprintdialogpage.h>

class TQSpinBox;
class TQComboBox;

class RichPage : public KPrintDialogPage
{
public:
	RichPage(TQWidget *parent = 0, const char *name = 0);
	~RichPage();

	void setOptions(const TQMap<TQString,TQString>& opts);
	void getOptions(TQMap<TQString,TQString>& opts, bool incldef = false);

private:
	QSpinBox	*margin_;
	QComboBox	*fontname_;
	QSpinBox	*fontsize_;
};

#endif
