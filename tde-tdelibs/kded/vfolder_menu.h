/* 
   This file is part of the KDE libraries
   Copyright (c) 2003 Waldo Bastian <bastian@kde.org>
   
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

#ifndef _VFOLDER_MENU_H_
#define _VFOLDER_MENU_H_

#include <tqobject.h>
#include <tqdom.h>
#include <tqstringlist.h>
#include <tqptrdict.h>
#include <tqptrlist.h>
#include <tqvaluestack.h>

#include <kservice.h>

class VFolderMenu : public TQObject
{
  TQ_OBJECT
public:
  class appsInfo;
  class SubMenu {
  public:
     SubMenu() : items(43),isDeleted(false),apps_info(0) { }
     ~SubMenu() { subMenus.setAutoDelete(true); }
  
  public:
     TQString name;
     TQString directoryFile;
     TQPtrList<SubMenu> subMenus;
     TQDict<KService> items;
     TQDict<KService> excludeItems; // Needed when merging due to Move.
     TQDomElement defaultLayoutNode;
     TQDomElement layoutNode;
     bool isDeleted;
     TQStringList layoutList;
     appsInfo *apps_info;
  };

  VFolderMenu();
  ~VFolderMenu();

  /**
   * Parses VFolder menu defintion and generates a menu layout.
   * The newService signals is used as callback to load
   * a specific service description.
   *
   * @param file Menu file to load
   * @param forceLegacyLoad flag indicating whether the KDE "applnk"
   * directory should be processed at least once.
   */
  SubMenu *parseMenu(const TQString &file, bool forceLegacyLoad=false);
  
  /**
   * Returns a list of all directories involved in the last call to 
   * parseMenu(), excluding the KDE Legacy directories.
   *
   * A change in any of these directories or in any of their child-
   * directories can result in changes to the menu.
   */
  TQStringList allDirectories();

  /**
   * Debug function to enable tracking of what happens with a specific
   * menu item id
   */
  void setTrackId(const TQString &id);

signals:
  void newService(const TQString &path, KService **entry);

public:
  struct MenuItem 
  {
    enum Type { MI_Service, MI_SubMenu, MI_Separator };
    Type type;
    union { 
       KService *service;
       SubMenu  *submenu;
    } data;
  };

public:  
  TQStringList m_allDirectories; // A list of all the directories that we touch

  TQStringList m_defaultDataDirs;
  TQStringList m_defaultAppDirs;
  TQStringList m_defaultDirectoryDirs;
  TQStringList m_defaultMergeDirs;
  TQStringList m_defaultLegacyDirs;

  TQStringList m_directoryDirs; // Current set of applicable <DirectoryDir> dirs
  TQDict<SubMenu> m_legacyNodes; // Dictionary that stores Menu nodes 
                                // associated with legacy tree.

  class docInfo {
  public:
     TQString baseDir; // Relative base dir of current menu file
     TQString baseName; // Filename of current menu file without ".menu"
     TQString path; // Full path of current menu file including ".menu"
  };
  

  docInfo m_docInfo; // docInfo for current doc
  TQValueStack<VFolderMenu::docInfo> m_docInfoStack;

  class appsInfo {
  public:
     appsInfo() : dictCategories(53), applications(997), appRelPaths(997)
     {
        dictCategories.setAutoDelete(true);
     }

     TQDict<KService::List> dictCategories; // category -> apps
     TQDict<KService> applications; // rel path -> service
     TQPtrDict<TQString> appRelPaths; // service -> rel path
  };
  
  appsInfo *m_appsInfo; // appsInfo for current menu
  TQPtrList<appsInfo> m_appsInfoStack; // All applicable appsInfo for current menu
  TQPtrList<appsInfo> m_appsInfoList; // List of all appsInfo objects.
  TQDict<KService> m_usedAppsDict; // all applications that have been allocated
  
  TQDomDocument m_doc;
  SubMenu *m_rootMenu;
  SubMenu *m_currentMenu;
  bool m_forcedLegacyLoad;
  bool m_legacyLoaded;
  bool m_track;
  TQString m_trackId;

private:
  /**
   * Lookup application by relative path
   */
  KService *findApplication(const TQString &relPath);

  /**
   * Lookup applications by category
   */
  TQPtrList<KService::List> findCategory(const TQString &category);
  
  /**
   * Add new application
   */
  void addApplication(const TQString &id, KService *service);
  
  /**
   * Build application indices
   */
  void buildApplicationIndex(bool unusedOnly);
  
  /**
   * Create a appsInfo frame for current menu
   */
  void createAppsInfo();

  /**
   * Load additional appsInfo frame for current menu
   */
  void loadAppsInfo();

  /**
   * Unload additional appsInfo frame for current menu
   */
  void unloadAppsInfo();

  TQDomDocument loadDoc();
  void mergeMenus(TQDomElement &docElem, TQString &name);
  void mergeFile(TQDomElement &docElem, const TQDomNode &mergeHere);
  void loadMenu(const TQString &filename);

  /**
   * Merge the items2 set into the items1 set
   */
  void includeItems(TQDict<KService> *items1, TQDict<KService> *items2);

  /**
   * Remove all items from the items1 set that aren't also in the items2 set
   */
  void matchItems(TQDict<KService> *items1, TQDict<KService> *items2);

  /**
   * Remove all items in the items2 set from the items1 set
   */
  void excludeItems(TQDict<KService> *items1, TQDict<KService> *items2);

  /**
   * Search the parentMenu tree for the menu menuName and takes it
   * out.
   *
   * This function returns a pointer to the menu if it was found 
   * or 0 if it was not found.
   */
  SubMenu* takeSubMenu(SubMenu *parentMenu, const TQString &menuName);

  /**
   * Insert the menu newMenu with name menuName into the parentMenu.
   * If such menu already exist the result is merged, if any additional
   * submenus are required they are created.
   * If reversePriority is false, newMenu has priority over the existing 
   * menu during merging.
   * If reversePriority is true, the existing menu has priority over newMenu
   * during merging.
   */
  void insertSubMenu(VFolderMenu::SubMenu *parentMenu, const TQString &menuName, VFolderMenu::SubMenu *newMenu, bool reversePriority=false);

  /**
   * Merge menu2 and it's submenus into menu1 and it's submenus
   * If reversePriority is false, menu2 has priority over menu1
   * If reversePriority is true, menu1 has priority over menu2
   */
  void mergeMenu(SubMenu *menu1, SubMenu *menu2, bool reversePriority=false);

  /**
   * Inserts service into the menu using name relative to parentMenu
   * Any missing sub-menus are created.
   */
  void insertService(SubMenu *parentMenu, const TQString &name, KService *newService);

  /**
   * Register the directory that @p file is in.
   * @see allDirectories()
   */
  void registerFile(const TQString &file);

  /**
   * Fill m_usedAppsDict with all applications from @p items
   */
  void markUsedApplications(TQDict<KService> *items);

  /**
   * Register @p directory
   * @see allDirectories()
   */
  void registerDirectory(const TQString &directory);

  void processKDELegacyDirs();
  void processLegacyDir(const TQString &dir, const TQString &relDir, const TQString &prefix);
  void processMenu(TQDomElement &docElem, int pass);
  void layoutMenu(VFolderMenu::SubMenu *menu, TQStringList defaultLayout);
  void processCondition(TQDomElement &docElem, TQDict<KService> *items);

  void initDirs();
  
  void pushDocInfo(const TQString &fileName, const TQString &baseDir = TQString::null);
  void pushDocInfoParent(const TQString &basePath, const TQString &baseDir);
  void popDocInfo();
  
  TQString absoluteDir(const TQString &_dir, const TQString &baseDir, bool keepRelativeToCfg=false);
  TQString locateMenuFile(const TQString &fileName); 
  TQString locateDirectoryFile(const TQString &fileName);
  void loadApplications(const TQString&, const TQString&);
};

#endif
