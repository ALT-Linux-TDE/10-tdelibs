#ifndef __TDEACCELPRIVATE_H
#define __TDEACCELPRIVATE_H

#include "kkeyserver_x11.h"
#include <tqtimer.h>

class TDEAccelAction;

/**
 * @internal
 */
class TDECORE_EXPORT TDEAccelPrivate : public TQObject, public TDEAccelBase
{
	TQ_OBJECT
 public:
	TDEAccel* m_pAccel;
	TQWidget* m_pWatch;
	TQMap<int, int> m_mapIDToKey;
	TQMap<int, TDEAccelAction*> m_mapIDToAction;
	TQTimer m_timerShowMenu;

	TDEAccelPrivate( TDEAccel* pParent, TQWidget* pWatch );

	virtual void setEnabled( bool bEnabled );

	bool setEnabled( const TQString& sAction, bool bEnable );

	virtual bool removeAction( const TQString& sAction );

	virtual bool emitSignal( TDEAccelBase::Signal signal );
	virtual bool connectKey( TDEAccelAction& action, const KKeyServer::Key& key );
	virtual bool connectKey( const KKeyServer::Key& key );
	virtual bool disconnectKey( TDEAccelAction& action, const KKeyServer::Key& key );
	virtual bool disconnectKey( const KKeyServer::Key& key );

 signals:
	void menuItemActivated();
	void menuItemActivated(TDEAccelAction*);

 private:
#ifndef TQ_WS_WIN /** @todo TEMP: new implementation (commit #424926) didn't work */
	void emitActivatedSignal(TDEAccelAction*);
#endif

 private slots:
	void slotKeyPressed( int id );
	void slotShowMenu();
	void slotMenuActivated( int iAction );
	
	bool eventFilter( TQObject* pWatched, TQEvent* pEvent ); // virtual method from TQObject
};

#endif // !__TDEACCELPRIVATE_H
