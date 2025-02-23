#include "notepad.h" // this plugin applies to a notepad part
#include <tqmultilineedit.h>
#include "plugin_spellcheck.h"
#include <tdeaction.h>
#include <kgenericfactory.h>
#include <tdemessagebox.h>
#include <tdelocale.h>
#include <kdebug.h>

PluginSpellCheck::PluginSpellCheck( TQObject* parent, const char* name, 
                                    const TQStringList& )
    : Plugin( parent, name )
{
    (void) new TDEAction( "&Select current line (plugin)", 0, this, TQ_SLOT(slotSpellCheck()),
                        actionCollection(), "tools-check-spelling" );
}

PluginSpellCheck::~PluginSpellCheck()
{
}

void PluginSpellCheck::slotSpellCheck()
{
    kdDebug() << "Plugin parent : " << parent()->name() << " (" << parent()->className() << ")" << endl;
    // The parent is assumed to be a NotepadPart
    if ( !parent()->inherits("NotepadPart") )
       KMessageBox::error(0L,"You just called the spell-check action on a wrong part (not NotepadPart)");
    else
    {
         NotepadPart * part = (NotepadPart *) parent();
         TQMultiLineEdit * widget = (TQMultiLineEdit *) part->widget();
         widget->selectAll(); //selects current line !
    }
}

K_EXPORT_COMPONENT_FACTORY( libspellcheckplugin, 
                            KGenericFactory<PluginSpellCheck> );

#include <plugin_spellcheck.moc>
