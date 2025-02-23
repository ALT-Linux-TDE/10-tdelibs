#ifndef _TDEACTIONSHORTCUTLIST_H
#define _TDEACTIONSHORTCUTLIST_H

#include <tdeshortcutlist.h>
#include <tdeaction.h>

//---------------------------------------------------------------------
// class TDEActionShortcutList
//---------------------------------------------------------------------

class TDEAccelShortcutListPrivate;
class TDEUI_EXPORT TDEActionShortcutList : public TDEShortcutList
{
 public:
	TDEActionShortcutList( TDEActionCollection* );
	virtual ~TDEActionShortcutList();

	virtual uint count() const;
	virtual TQString name( uint index ) const;
	virtual TQString label( uint index ) const;
	virtual TQString whatsThis( uint index ) const;
	virtual const TDEShortcut& shortcut( uint index ) const;
	virtual const TDEShortcut& shortcutDefault( uint index ) const;
	virtual bool isConfigurable( uint index ) const;
	virtual bool setShortcut( uint index, const TDEShortcut& shortcut );

	virtual const TDEInstance* instance() const;

	virtual TQVariant getOther( Other, uint index ) const;
	virtual bool setOther( Other, uint index, TQVariant );

	virtual bool save() const;

	const TDEAction *action( uint ) const;

 protected:
	TDEActionCollection& m_actions;

 protected:
        virtual void virtual_hook( int id, void* data );
 private:
	TDEAccelShortcutListPrivate* d;
};

//---------------------------------------------------------------------
// class TDEActionPtrShortcutList
//---------------------------------------------------------------------

class TDEAccelShortcutListPrivate;
class TDEUI_EXPORT TDEActionPtrShortcutList : public TDEShortcutList
{
 public:
	TDEActionPtrShortcutList( TDEActionPtrList& );
	virtual ~TDEActionPtrShortcutList();

	virtual uint count() const;
	virtual TQString name( uint index ) const;
	virtual TQString label( uint index ) const;
	virtual TQString whatsThis( uint index ) const;
	virtual const TDEShortcut& shortcut( uint index ) const;
	virtual const TDEShortcut& shortcutDefault( uint index ) const;
	virtual bool isConfigurable( uint index ) const;
	virtual bool setShortcut( uint index, const TDEShortcut& shortcut);

	virtual TQVariant getOther( Other, uint index ) const;
	virtual bool setOther( Other, uint index, TQVariant );

	virtual bool save() const;

 protected:
	TDEActionPtrList& m_actions;

 protected:
       virtual void virtual_hook( int id, void* data );
 private:
	TDEAccelShortcutListPrivate* d;
};

#endif // !_TDEACTIONSHORTCUTLIST_H
