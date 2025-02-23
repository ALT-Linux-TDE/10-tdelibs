#ifndef DRAWDEMO_H
#define DRAWDEMO_H

//
// DrawView has installable draw routines, just add a function pointer
// and a text in the table above.
//

class DrawView : public TQWidget
{
    TQ_OBJECT
public:
    DrawView();
    ~DrawView();
public slots:
    void   updateIt( int );
    void   printIt();
protected:
    void   drawIt( TQPainter * );
    void   paintEvent( TQPaintEvent * );
    void   resizeEvent( TQResizeEvent * );
private:
    KPrinter	 *printer;
    TQButtonGroup *bgroup;
    QPushButton	 *print;
    int		  drawindex;
    int		  maxindex;
};

#endif
