/*
 * $Id$
 */

#ifndef _KTABCTLTEST_H
#define _KTABCTLTEST_H

#include <tqwidget.h>
class KTabCtl;
class TQPushButton;
class TQMultiLineEdit;

class TopLevel : public TQWidget
{
    TQ_OBJECT
public:

    TopLevel( TQWidget *parent=0, const char *name=0 );
protected:
    void resizeEvent(TQResizeEvent *);
    KTabCtl *test;
    TQPushButton *ok, *cancel;
    TQMultiLineEdit *e;
    TQWidget *pages[3];
public slots:
    void okPressed();
    void tabChanged(int);
};

#endif
