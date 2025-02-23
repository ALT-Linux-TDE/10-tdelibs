/* This file is part of the KDE libraries
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2000 Scott Manson <sdmanson@alltel.net>

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

#ifndef __KATE_SYNTAXDOCUMENT_H__
#define __KATE_SYNTAXDOCUMENT_H__

#include <tqdom.h>
#include <tqstringlist.h>

/**
 * Information about each syntax hl Mode
 */
class KateSyntaxModeListItem
{
  public:
    TQString name;
    TQString nameTranslated;
    TQString section;
    TQString mimetype;
    TQString extension;
    TQString identifier;
    TQString version;
    TQString priority;
    TQString author;
    TQString license;
    bool hidden;
};

/**
 * List of the KateSyntaxModeListItems holding all the syntax mode list items
 */
typedef TQValueList<KateSyntaxModeListItem*> KateSyntaxModeList;

/**
 * Class holding the data around the current QDomElement
 */
class KateSyntaxContextData
{
  public:
    TQDomElement parent;
    TQDomElement currentGroup;
    TQDomElement item;
};

/**
 * Store and manage the information about Syntax Highlighting.
 */
class KateSyntaxDocument : public TQDomDocument
{
  public:
    /**
     * Constructor
     * Sets the current file to nothing and build the ModeList (katesyntaxhighlightingrc)
     * @param force fore the update of the hl cache
     */
    KateSyntaxDocument(bool force = false);

    /**
     * Desctructor
     */
    ~KateSyntaxDocument();

    /**
     * If the open hl file is different from the one needed, it opens
     * the new one and assign some other things.
     * @param identifier file name and path of the new xml needed
     * @return success
     */
    bool setIdentifier(const TQString& identifier);

    /**
     * Get the mode list
     * @return mode list
     */
    const KateSyntaxModeList &modeList() { return myModeList; }

    /**
     * Jump to the next group, KateSyntaxContextData::currentGroup will point to the next group
     * @param data context
     * @return success
     */
    bool nextGroup(KateSyntaxContextData* data);

    /**
     * Jump to the next item, KateSyntaxContextData::item will point to the next item
     * @param data context
     * @return success
     */
    bool nextItem(KateSyntaxContextData* data);

    /**
     * This function is used to fetch the atributes of the tags.
     */
    TQString groupItemData(const KateSyntaxContextData* data,const TQString& name);
    TQString groupData(const KateSyntaxContextData* data,const TQString& name);

    void freeGroupInfo(KateSyntaxContextData* data);
    KateSyntaxContextData* getSubItems(KateSyntaxContextData* data);

    /**
     * Get the KateSyntaxContextData of the DomElement Config inside mainGroupName
     * It just fills KateSyntaxContextData::item
     */
    KateSyntaxContextData* getConfig(const TQString& mainGroupName, const TQString &config);

    /**
     * Get the KateSyntaxContextData of the TQDomElement Config inside mainGroupName
     * KateSyntaxContextData::parent will contain the TQDomElement found
     */
    KateSyntaxContextData* getGroupInfo(const TQString& mainGroupName, const TQString &group);

    /**
     * Returns a list with all the keywords inside the list type
     */
    TQStringList& finddata(const TQString& mainGroup,const TQString& type,bool clearList=true);

  private:
    /**
     * Generate the list of hl modes, store them in myModeList
     * @param force if true forces to rebuild the Mode List from the xml files (instead of katesyntax...rc)
     */
    void setupModeList(bool force);

    /**
     * Used by getConfig and getGroupInfo to traverse the xml nodes and
     * evenually return the found element
     */
    bool getElement (TQDomElement &element, const TQString &mainGroupName, const TQString &config);

    /**
     * List of mode items
     */
    KateSyntaxModeList myModeList;

    /**
     * current parsed filename
     */
    TQString currentFile;

    /**
     * last found data out of the xml
     */
    TQStringList m_data;
};

#endif
