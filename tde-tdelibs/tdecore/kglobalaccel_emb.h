#ifndef _KGLOBALACCEL_EMB_H
#define _KGLOBALACCEL_EMB_H

#include "tdeaccelbase.h"
#include "tdeshortcut.h"

class TDEGlobalAccelPrivate
{
public:
	TDEGlobalAccelPrivate();

	virtual void setEnabled( bool bEnabled );

	virtual bool connectKey( TDEAccelAction&, KKeySequence );
	virtual bool disconnectKey( TDEAccelAction&, KKeySequence );
};

#endif // _KGLOBALACCEL_EMB_H
