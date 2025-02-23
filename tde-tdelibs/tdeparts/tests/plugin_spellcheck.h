#ifndef __plugin_spellcheck_h
#define __plugin_spellcheck_h

#include <tdeparts/plugin.h>

class PluginSpellCheck : public KParts::Plugin
{
    TQ_OBJECT
public:
    PluginSpellCheck( TQObject* parent = 0, const char* name = 0, 
                      const TQStringList& = TQStringList() );
    virtual ~PluginSpellCheck();

public slots:
    void slotSpellCheck();
};

#endif
