#include <tdeapplication.h>
#include <keditlistbox.h>
#include <kcombobox.h>

int main( int argc, char **argv )
{
    TDEApplication app( argc, argv, "keditlistboxtest" );

    KEditListBox::CustomEditor editor( new KComboBox( true, 0L, "test" ) );
    KEditListBox *box = new KEditListBox( TQString::fromLatin1("KEditListBox"),
                                          editor );
    
    box->insertItem( TQString::fromLatin1("Test") );
    box->insertItem( TQString::fromLatin1("for") );
    box->insertItem( TQString::fromLatin1("this") );
    box->insertItem( TQString::fromLatin1("KEditListBox") );
    box->insertItem( TQString::fromLatin1("Widget") );
    box->show();

    return app.exec();
}
