/*

(C) Daniel M. Duley <mosfet@kde.org>
(C) Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "kpanelappmenu.h"
#include <tqstringlist.h>
#include <dcopclient.h>
#include <tdeapplication.h>
#include <kdebug.h>

static int panelmenu_get_seq_id()
{
    static int panelmenu_seq_no = -2;
    return panelmenu_seq_no--;
}


KPanelAppMenu::KPanelAppMenu(const TQString &title, TQObject *parent,
                       const char *name)
    : TQObject(parent, name), DCOPObject()
{
    init(TQString(), title);
}

KPanelAppMenu::KPanelAppMenu(const TQPixmap &icon, const TQString &title,
                       TQObject *parent, const char *name)
: TQObject(parent, name), DCOPObject()
{

    init(icon, title);
}


KPanelAppMenu::KPanelAppMenu(TQObject *parent, const char *name)
  : TQObject(parent, name), DCOPObject(name)
{
  realObjId = name;
}


void KPanelAppMenu::init(const TQPixmap &icon, const TQString &title)
{
    DCOPClient *client = kapp->dcopClient();
    if(!client->isAttached())
	client->attach();
    TQByteArray sendData, replyData;
    TQCString replyType;
    {
	TQDataStream stream(sendData, IO_WriteOnly);
	stream << icon << title;
	if ( client->call("kicker", "kickerMenuManager", "createMenu(TQPixmap,TQString)", sendData, replyType, replyData ) ) {
	  if (replyType != "TQCString")
	    kdDebug() << "error! replyType for createMenu should be QCstring in KPanelAppMenu::init" << endl;
	  else {
	    TQDataStream reply( replyData, IO_ReadOnly );
	    reply >> realObjId;
	  }
	}
    }
    {
	TQDataStream stream(sendData, IO_WriteOnly);
	stream << TQCString("activated(int)") << client->appId() << objId();
	client->send("kicker", realObjId, "connectDCOPSignal(TQCString,TQCString,TQCString)", sendData);
    }
}

KPanelAppMenu::~KPanelAppMenu()
{
    DCOPClient *client = kapp->dcopClient();
    TQByteArray sendData;
    TQDataStream stream(sendData, IO_WriteOnly);
    stream << realObjId;
    client->send("kicker", "kickerMenuManager", "removeMenu", sendData );
}

int KPanelAppMenu::insertItem(const TQPixmap &icon, const TQString &text, int id )
{
    if ( id < 0 )
	id = panelmenu_get_seq_id();
    DCOPClient *client = kapp->dcopClient();
    TQByteArray sendData;
    TQDataStream stream(sendData, IO_WriteOnly);
    stream << icon << text << id;
    client->send("kicker", realObjId, "insertItem(TQPixmap,TQString,int)", sendData );
    return id;
}


KPanelAppMenu *KPanelAppMenu::insertMenu(const TQPixmap &icon, const TQString &text, int id )
{
    if ( id < 0 )
        id = panelmenu_get_seq_id();
    DCOPClient *client = kapp->dcopClient();
    TQByteArray sendData, replyData;
    TQCString replyType;
    TQDataStream stream(sendData, IO_WriteOnly);
    stream << icon << text << id;
    client->call("kicker", realObjId, "insertMenu(TQPixmap,TQString,int)", sendData, replyType, replyData );
    if ( replyType != "TQCString")
      return 0;
    TQDataStream ret(replyData, IO_ReadOnly);
    TQCString subid;
    ret >> subid;

    TQByteArray sendData2;
    TQDataStream stream2(sendData2, IO_WriteOnly);
    stream2 << TQCString("activated(int)") << client->appId() << subid;
    client->send("kicker", subid, "connectDCOPSignal(TQCString,TQCString,TQCString)", sendData2);

    return new KPanelAppMenu(this, subid);
}


int KPanelAppMenu::insertItem(const TQString &text, int id )
{
    if ( id < 0 )
	id = panelmenu_get_seq_id();
    DCOPClient *client = kapp->dcopClient();
    TQByteArray sendData;
    TQDataStream stream(sendData, IO_WriteOnly);
    stream << text << id;
    client->send("kicker", realObjId, "insertItem(TQString,int)", sendData );
    return id;
}


void KPanelAppMenu::clear()
{
    DCOPClient *client = kapp->dcopClient();
    TQByteArray sendData;
    client->send("kicker", realObjId, "clear()", sendData);
}


bool KPanelAppMenu::process(const TQCString &fun, const TQByteArray &data,
			 TQCString &replyType, TQByteArray &)
{
    if ( fun == "activated(int)" ) {
	TQDataStream dataStream( data, IO_ReadOnly );
	int id;
	dataStream >> id;
	emit activated( id );
	replyType = "void";
	return true;
    }
    return false;
}


#include "kpanelappmenu.moc"












