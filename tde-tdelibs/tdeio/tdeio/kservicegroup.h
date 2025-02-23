/* This file is part of the KDE project
   Copyright (C) 2000 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __kservicegroup_h__
#define __kservicegroup_h__

#include <tqptrlist.h>
#include <tqstring.h>
#include <tqshared.h>
#include <tqdatastream.h>
#include <tqvariant.h>

#include <kdesktopfile.h>
#include <ksortablevaluelist.h>

#include "tdesycocaentry.h"
#include "tdesycocatype.h"
#include "kservice.h"

class KBuildServiceGroupFactory;

/**
 * KServiceGroup represents a group of service, for example
 * screensavers.
 * This class is typically used like this:
 *
 * \code
 * // Lookup screensaver group
 * KServiceGroup::Ptr group = KServiceGroup::baseGroup("screensavers");
 * if (!group || !group->isValid()) return;
 *
 * KServiceGroup::List list = group->entries();
 *
 * // Iterate over all entries in the group
 * for( KServiceGroup::List::ConstIterator it = list.begin();
 *      it != list.end(); it++)
 * {
 *    KSycocaEntry *p = (*it);
 *    if (p->isType(KST_KService))
 *    {
 *       KService *s = static_cast<KService *>(p);
 *       printf("Name = %s\n", s->name().latin1());
 *    }
 *    else if (p->isType(KST_KServiceGroup))
 *    {
 *       KServiceGroup *g = static_cast<KServiceGroup *>(p);
 *       // Sub group ...
 *    }
 * }
 * \endcode
 * @short Represents a group of services
 */
class TDEIO_EXPORT KServiceGroup : public KSycocaEntry
{
  friend class KBuildServiceGroupFactory;
  K_SYCOCATYPE( KST_KServiceGroup, KSycocaEntry )

public:
  typedef TDESharedPtr<KServiceGroup> Ptr;
  typedef TDESharedPtr<KSycocaEntry> SPtr;
  typedef TQValueList<SPtr> List;
public:
  /**
   * Construct a dummy servicegroup indexed with @p name.
   * @param name the name of the service group
   * @since 3.1
   */
  KServiceGroup( const TQString & name );

  /**
   * Construct a service and take all informations from a config file
   * @param _fullpath full path to the config file
   * @param _relpath relative path to the config file
   */
  KServiceGroup( const TQString & _fullpath, const TQString & _relpath );

  /**
   * @internal construct a service from a stream.
   * The stream must already be positionned at the correct offset
   */
  KServiceGroup( TQDataStream& _str, int offset, bool deep );

  virtual ~KServiceGroup();

  /**
   * Checks whether the entry is valid, returns always true.
   * @return true
   */
  bool isValid() const { return true; }

  /**
   * Name used for indexing.
   * @return the service group's name
   */
  virtual TQString name() const { return entryPath(); }

  /**
   * Returns the relative path of the service group.
   * @return the service group's relative path
   */
  virtual TQString relPath() const { return entryPath(); }

  /**
   * Returns the caption of this group.
   * @return the caption of this group
   */
  TQString caption() const { return m_strCaption; }

  /**
   * Returns the name of the icon associated with the group.
   * @return the name of the icon associated with the group,
   *         or TQString::null if not set
   */
  TQString icon() const { return m_strIcon; }

  /**
   * Returns the comment about this service group.
   * @return the descriptive comment for the group, if there is one,
   *         or TQString::null if not set
   */
  TQString comment() const { return m_strComment; }

  /**
   * Returns the total number of displayable services in this group and
   * any of its subgroups.
   * @return the number of child services
   */
  int childCount();

  /**
   * Returns true if the NoDisplay flag was set, i.e. if this
   * group should be hidden from menus, while still being in tdesycoca.
   * @return true to hide this service group, false to display it
   * @since 3.1
   */
  bool noDisplay() const;

  /**
   * Return true if we want to display empty menu entry
   * @return true to show this service group as menu entry is empty, false to hide it
   * @since 3.4
   */
  bool showEmptyMenu() const;
  void setShowEmptyMenu( bool b);

  /**
   * @return true to show an inline header into menu
   * @since 3.5
   */
  bool showInlineHeader() const;
  void setShowInlineHeader(bool _b);

  /**
   * @return true to show an inline alias item into menu
   * @since 3.5
   */
  bool inlineAlias() const;
  void setInlineAlias(bool _b);
  /**
   * @return true if we allow to inline menu.
   * @since 3.5
   */
  bool allowInline() const;
  void setAllowInline(bool _b);

  /**
   * @return inline limite value
   * @since 3.5
   */
  int inlineValue() const;
  void setInlineValue(int _val);


  /**
   * Returns a list of untranslated generic names that should be
   * be supressed when showing this group.
   * E.g. The group "Games/Arcade" might want to suppress the generic name
   * "Arcade Game" since it's redundant in this particular context.
   * @since 3.2
   */
  TQStringList suppressGenericNames() const;

  /**
   * @internal
   * Sets information related to the layout of services in this group.
   */
  void setLayoutInfo(const TQStringList &layout);

  /**
   * Original API and feature kindly provided by SuSE
   */
  bool SuSEshortMenu() const;
  bool SuSEgeneralDescription() const;

  /**
   * @internal
   * Returns information related to the layout of services in this group.
   */
  TQStringList layoutInfo() const;

  /**
   * @internal
   * Load the service from a stream.
   */
  virtual void load( TQDataStream& );
  /**
   * @internal
   * Save the service to a stream.
   */
  virtual void save( TQDataStream& );

  /**
   * List of all Services and ServiceGroups within this
   * ServiceGroup.
   * @param sorted true to sort items
   * @param excludeNoDisplay true to exclude items marked "NoDisplay"
   * @param allowSeparators true to allow separator items to be included
   * @param sortByGenericName true to sort GenericName+Name instead of Name+GenericName
   * @return the list of entries
   * @since 3.2
   */
  List entries(bool sorted, bool excludeNoDisplay, bool allowSeparators, bool sortByGenericName=false);
  virtual List entries(bool sorted, bool excludeNoDisplay);

  /**
   * List of all Services and ServiceGroups within this
   * ServiceGroup.
   * @param sorted true to sort items
   * @return the list of entried
   */
  virtual List entries(bool sorted = false);

  /*
   * Original API and feature kindly provided by SuSE
   */
  virtual List SuSEentries(bool sort, bool excludeNoDisplay, bool allowSeparators, bool sortByGenericName, bool excludeSuSEunimportant = false);
  virtual List SuSEsortEntries( KSortableValueList<SPtr,TQCString> slist, KSortableValueList<SPtr,TQCString> glist, bool excludeNoDisplay, bool allowSeparators );

  /**
   * Returns a non-empty string if the group is a special base group.
   * By default, "Settings/" is the kcontrol base group ("settings")
   * and "System/Screensavers/" is the screensavers base group ("screensavers").
   * This allows moving the groups without breaking those apps.
   *
   * The base group is defined by the X-TDE-BaseGroup key
   * in the .directory file.
   * @return the base group name, or null if no base group
   */
  TQString baseGroupName() const { return m_strBaseGroupName; }

  /**
   * Returns a path to the .directory file describing this service group.
   * The path is either absolute or relative to the "apps" resource.
   * @since 3.2
   */
  TQString directoryEntryPath() const;

  /**
   * Returns the group for the given baseGroupName.
   * Can return 0L if the directory (or the .directory file) was deleted.
   * @return the base group with the given name, or 0 if not available.
   */
  static Ptr baseGroup( const TQString &baseGroupName );

  /**
   * Returns the root service group.
   * @return the root service group
   */
  static Ptr root();

  /**
   * Returns the group with the given relative path.
   * @param relPath the path of the service group
   * @return the group with the given relative path name.
   */
  static Ptr group(const TQString &relPath);

  /**
   * Returns the group of services that have X-TDE-ParentApp equal
   * to @p parent (siblings).
   * @param parent the name of the service's parent
   * @return the services group
   * @since 3.1
   */
  static Ptr childGroup(const TQString &parent);

  /**
   * This function parse attributes into menu
   * @since 3.5
   */
    void parseAttribute( const TQString &item ,  bool &showEmptyMenu, bool &showInline, bool &showInlineHeader, bool & showInlineAlias ,int &inlineValue );

protected:
  /**
   * @internal
   * Add a service to this group
   */
  void addEntry( KSycocaEntry *entry);

  TQString m_strCaption;
  TQString m_strIcon;
  TQString m_strComment;

  List m_serviceList;
  bool m_bDeep;
  TQString m_strBaseGroupName;
  int m_childCount;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class Private;
  Private* d;
};

class TDEIO_EXPORT KServiceSeparator : public KSycocaEntry
{
  K_SYCOCATYPE( KST_KServiceSeparator, KSycocaEntry )

public:
  typedef TDESharedPtr<KServiceSeparator> Ptr;
public:
  /**
   * Construct a service separator
   * @since 3.2
   */
  KServiceSeparator();

  bool isValid() const { return true; }

  // Dummy
  virtual TQString name() const { return "separator"; }
  // Dummy
  virtual void load( TQDataStream& ) { };
  // Dummy
  virtual void save( TQDataStream& ) { };
};

#endif
