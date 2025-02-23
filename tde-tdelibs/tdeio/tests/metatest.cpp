#include <tdeapplication.h>
#include <tdefilemetainfo.h>
#include <tdecmdlineargs.h>
#include <tqstringlist.h>
#include <tqimage.h>
#include <kdebug.h>
#include <tqlabel.h>
#include <tqvalidator.h>

#define I18N_NOOP

 static TDECmdLineOptions options[] = 
  {
     { "+file", "File name", 0 },
     { "addgroup ", "Add a group to a file", 0},
     { "removegroup ", "Remove a group from a file", 0},
     { "removeitem ", "Remove the item from --group from a file", 0},
     { "group ", "Specify a group to work on", 0},
     { "validator", "Create a validator for an item and show its class", 0},
     { "item ", "Specify an item to work on", 0},
     { "add ", "Add the --item to the --group and set the value", 0},
     { "autogroupadd", "Automatically add a group if none is found", 0},
     { "set ", "Set the value of --item in --group", 0},
     { "groups",  "list the groups of this file", 0 },
     { "mimetypeinfo ", "the mimetype info for a mimetype", 0 },
     TDECmdLineLastOption
  };
  
void printKeyValues(KFileMetaInfo& info)
{    
    TQStringList l = info.preferredKeys();
    kdDebug() << "found " << l.size() << " keys\n";
    
    TQString s;
    TQStringList::Iterator it;
    for (it = l.begin(); it!=l.end(); ++it)
    {
        s +=" - " + *it;
    }
    kdDebug() << "keys: " << s << endl;
    
    for (it = l.begin(); it!=l.end(); ++it)
    {
        KFileMetaInfoItem item = info.item(*it);
        if ( item.isValid() && item.value().canCast(TQVariant::String)) {
            kdDebug() << item.key() << "(" << item.translatedKey() << ") -> "
                      << item.string() << endl;
        }
    }
}
  
void printMimeTypeInfo(TQString mimetype)
{
    const KFileMimeTypeInfo* info = KFileMetaInfoProvider::self()->mimeTypeInfo(mimetype);

    if (!info) return;
    kdDebug() << "Preferred groups:\n";
    kdDebug() << "=================\n";
    TQStringList groups = info->preferredGroups();

    for (TQStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
    {
        kdDebug() << *it << endl;
    }

    kdDebug() << endl;
    kdDebug() << "Supported groups:\n";
    kdDebug() << "=================\n";
    groups = info->supportedGroups();
    for (TQStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
    {
        kdDebug() << *it << endl;
    }
        
    for (TQStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
    {
        const KFileMimeTypeInfo::GroupInfo* groupinfo = info->groupInfo(*it);

        kdDebug() << endl;
        kdDebug() << "Group \"" << *it << "\"\n";
        kdDebug() << "==================\n";

        if (!groupinfo) kdDebug() << "argh! no group info\n";

        kdDebug() << endl;
        kdDebug() << "  Supported keys:\n";
        TQStringList keys = groupinfo->supportedKeys();
        if (!keys.count()) kdDebug() << "  none\n";
        for (TQStringList::Iterator kit=keys.begin(); kit!=keys.end(); ++kit)
        {
            kdDebug() << "  " << *kit << endl;
                
            const KFileMimeTypeInfo::ItemInfo* iti = groupinfo->itemInfo(*kit);
            kdDebug() << "    Key:        " << iti->key() << endl;
            kdDebug() << "    Translated: " << iti->key() << endl;
            kdDebug() << "    Type:       " << TQVariant::typeToName(iti->type()) << endl;
            kdDebug() << "    Unit:       " << iti->unit() << endl;
            kdDebug() << "    Hint:       " << iti->hint() << endl;
            kdDebug() << "    Attributes: " << iti->attributes() << endl;
            kdDebug() << "    Prefix:     " << iti->prefix() << endl;
            kdDebug() << "    Suffix:     " << iti->suffix() << endl;
        }
            
        kdDebug() << "  name:       " << groupinfo->name() << endl;
        kdDebug() << "  translated: " << groupinfo->translatedName() << endl;
        kdDebug() << "  attributes: " << groupinfo->attributes() << endl;
        kdDebug() << "  variable keys: " << (groupinfo->supportsVariableKeys() ? "Yes" : "No") << endl;
        if (groupinfo->supportsVariableKeys())
        {
            const KFileMimeTypeInfo::ItemInfo* iti = groupinfo->variableItemInfo();
            kdDebug() << "  variable key type/attr: " << TQVariant::typeToName(iti->type()) << " / " << iti->attributes() << endl;
        }
    }
        
    kdDebug() << endl;
    kdDebug() << "Preferred keys:\n";
    kdDebug() << "===============\n";
    TQStringList prefKeys = info->preferredKeys();
    if (!prefKeys.count()) kdDebug() << "  none\n";
    for (TQStringList::Iterator kit=prefKeys.begin(); kit!=prefKeys.end(); ++kit)
    {
        kdDebug() << *kit << endl;
    }

    kdDebug() << endl;
    kdDebug() << "Supported keys:\n";
    kdDebug() << "===============\n";
    TQStringList supKeys = info->supportedKeys();
    if (!supKeys.count()) kdDebug() << "  none\n";
    for (TQStringList::Iterator kit=supKeys.begin(); kit!=supKeys.end(); ++kit)
    {
        kdDebug() << *kit << endl;
    }
}

void addGroup(KFileMetaInfo& info, TQString group)
{
    kdDebug() << "trying to add group " << group << endl;

    kdDebug() << "groups before: \n";
    TQStringList groups = info.groups();
    for (TQStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
        kdDebug() << "  " << *it << endl;
      
    if (info.addGroup(group))
       kdDebug() << "addGroup succeeded\n";
    else
       kdDebug() << "addGroup failed\n";
    
    kdDebug() << "trying another addGroup to see what happens\n";
    
    if (info.addGroup(group))
       kdDebug() << "addGroup succeeded\n";
    else
       kdDebug() << "addGroup failed\n";
    
        
    kdDebug() << "and afterwards: \n";
    groups = info.groups();
    for (TQStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
        kdDebug() << "  " << *it << endl;
}

void removeGroup(KFileMetaInfo& info, TQString group)
{
    kdDebug() << "trying to remove group " << group << endl;

    kdDebug() << "groups before: \n";
    TQStringList groups = info.groups();
    for (TQStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
        kdDebug() << "  " << *it << endl;
      
    info.removeGroup(group);
        
    kdDebug() << "and afterwards: \n";
    groups = info.groups();
    for (TQStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
        kdDebug() << "  " << *it << endl;
}
  
int main( int argc, char **argv )
{
    // Initialize command line args
    TDECmdLineArgs::init(argc, argv, "tdefilemetatest", "testing kfilmetainfo", "X");

    // Tell which options are supported
    TDECmdLineArgs::addCmdLineOptions( options );

    // Add options from other components
    TDEApplication::addCmdLineOptions();
    
    TDEApplication app;

    TDECmdLineArgs* args = TDECmdLineArgs::parsedArgs();

    TQCString ov;
    ov = args->getOption("mimetypeinfo");
    if (ov)
    {
        printMimeTypeInfo(ov);
        return 0;
    }
    
    if (!args->count()) return 1;

    KFileMetaInfo info( args->url(0), TQString::null, KFileMetaInfo::Everything);
    
    if (args->isSet("groups"))
    {
        TQStringList groups = info.groups();
        for (TQStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
        {
            kdDebug() << "group " << *it << endl;
        }
        return 0;
    }
    
    TQString group, item;

    ov = args->getOption("addgroup");
    if (ov) addGroup(info, ov);

    ov = args->getOption("removegroup");
    if (ov) removeGroup(info, ov);
    
    ov = args->getOption("group");
    if (ov) group = ov;

    ov = args->getOption("item");
    if (ov) item = ov;
    
    ov = args->getOption("add");
    if (ov && !group.isNull() && !item.isNull())
    {
        KFileMetaInfoGroup g = info[group];
        if (!g.isValid() && args->isSet("autogroupadd"))
        {
            kdDebug() << "group is not there, adding it\n";
            info.addGroup(group);
        }
        // add the item
        KFileMetaInfoItem i = info[group].addItem(item);
        if (i.isValid())
            kdDebug() << "additem success\n";
        else
            kdDebug() << "additem failed\n";

        if (i.setValue(ov))
            kdDebug() << "setValue success\n";
        else
            kdDebug() << "setValue failed\n";
    }
    
    ov = args->getOption("set");
    if (ov && !group.isNull() && !item.isNull())
    {
        if (info[group][item].setValue(TQString(ov)))
            kdDebug() << "setValue success\n";
        else
            kdDebug() << "setValue failed\n";
    }
    
    ov = args->getOption("removeitem");
    if (ov && !group.isNull())
    {
        if (info[group].removeItem(ov))
            kdDebug() << "removeitem success\n";
        else
            kdDebug() << "removeitem failed\n";
    }

    if (args->isSet("validator") && !group.isNull() && !item.isNull())
    {
        const KFileMimeTypeInfo* kfmti = KFileMetaInfoProvider::self()->mimeTypeInfo(info.mimeType());
        TQValidator* v = kfmti->createValidator(group, item);
        if (!v)
            kdDebug() << "got no validator\n";
        else
        {
            kdDebug() << "validator is a " << v->className() << endl;
            delete v;
        }
        
    }
    
    kdDebug() << "is it valid?\n";

    if (!info.isValid()) return 1;

    kdDebug() << "it is!\n";

    printKeyValues(info);

      
    kdDebug() << "========= again after applyChanges() =========\n";
    
    info.applyChanges();
    
    printKeyValues(info);

    KFileMetaInfoItem thumbitem = info.item(KFileMimeTypeInfo::Thumbnail);
//    KFileMetaInfoItem thumbitem = info.item("Thumbnail");
    
    if (!thumbitem.isValid()) kdDebug() << "no thumbnail\n";
    else
        kdDebug() << "type of thumbnail is " << thumbitem.value().typeName() << endl;
    
    
    if (thumbitem.isValid() && thumbitem.value().canCast(TQVariant::Image))
    {
        TQLabel* label = new TQLabel(0);
        app.setMainWidget(label);
        TQPixmap pix;
        pix.convertFromImage(thumbitem.value().toImage());
        label->setPixmap(pix);
        label->show();
        app.exec();
    }
    
    return 0;
}
