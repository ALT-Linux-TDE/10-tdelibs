/* This file is part of the KDE libraries
   Copyright (C) 2001,2002 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

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

#ifndef __KATE_HIGHLIGHT_H__
#define __KATE_HIGHLIGHT_H__

#include "katetextline.h"
#include "kateattribute.h"

#include "../interfaces/document.h"

#include <tdeconfig.h>

#include <tqptrlist.h>
#include <tqvaluelist.h>
#include <tqvaluevector.h>
#include <tqregexp.h>
#include <tqdict.h>
#include <tqintdict.h>
#include <tqmap.h>
#include <tqobject.h>
#include <tqstringlist.h>
#include <tqguardedptr.h>
#include <tqdatetime.h>
#include <tqpopupmenu.h>

class KateHlContext;
class KateHlItem;
class KateHlItemData;
class KateHlData;
class KateEmbeddedHlInfo;
class KateHlIncludeRule;
class KateSyntaxDocument;
class KateTextLine;
class KateSyntaxModeListItem;
class KateSyntaxContextData;

// some typedefs
typedef TQPtrList<KateAttribute> KateAttributeList;
typedef TQValueList<KateHlIncludeRule*> KateHlIncludeRules;
typedef TQPtrList<KateHlItemData> KateHlItemDataList;
typedef TQPtrList<KateHlData> KateHlDataList;
typedef TQMap<TQString,KateEmbeddedHlInfo> KateEmbeddedHlInfos;
typedef TQMap<int*,TQString> KateHlUnresolvedCtxRefs;
typedef TQValueList<int> IntList;

//Item Properties: name, Item Style, Item Font
class KateHlItemData : public KateAttribute
{
  public:
    KateHlItemData(const TQString  name, int defStyleNum);

    enum ItemStyles {
      dsNormal,
      dsKeyword,
      dsDataType,
      dsDecVal,
      dsBaseN,
      dsFloat,
      dsChar,
      dsString,
      dsComment,
      dsOthers,
      dsAlert,
      dsFunction,
      dsRegionMarker,
      dsError };

  public:
    const TQString name;
    int defStyleNum;
};

class KateHlData
{
  public:
    KateHlData(const TQString &wildcards, const TQString &mimetypes,const TQString &identifier, int priority);

  public:
    TQString wildcards;
    TQString mimetypes;
    TQString identifier;
    int priority;
};

class KateHighlighting
{
  public:
    KateHighlighting(const KateSyntaxModeListItem *def);
    ~KateHighlighting();

  public:
    void doHighlight ( KateTextLine *prevLine,
                       KateTextLine *textLine,
                       TQMemArray<uint> *foldingList,
                       bool *ctxChanged );

    void loadWildcards();
    TQValueList<TQRegExp>& getRegexpExtensions();
    TQStringList& getPlainExtensions();

    TQString getMimetypes();

    // this pointer needs to be deleted !!!!!!!!!!
    KateHlData *getData();
    void setData(KateHlData *);

    void setKateHlItemDataList(uint schema, KateHlItemDataList &);

    // both methodes return hard copies of the internal lists
    // the lists are cleared first + autodelete is set !
    // keep track that you delete them, or mem will be lost
    void getKateHlItemDataListCopy (uint schema, KateHlItemDataList &);

    const TQString &name() const {return iName;}
    const TQString &nameTranslated() const {return iNameTranslated;}
    const TQString &section() const {return iSection;}
    bool hidden() const {return iHidden;}
    const TQString &version() const {return iVersion;}
    const TQString &author () const { return iAuthor; }
    const TQString &license () const { return iLicense; }
    int priority();
    const TQString &getIdentifier() const {return identifier;}
    void use();
    void release();

    /**
     * @return true if the character @p c is not a deliminator character
     *     for the corresponding highlight.
     */
    bool isInWord( TQChar c, int attrib=0 ) const;

    /**
     * @return true if the character @p c is a wordwrap deliminator as specified
     * in the general keyword section of the xml file.
     */
    bool canBreakAt( TQChar c, int attrib=0 ) const;

    /**
    * @return true if @p beginAttr and @p endAttr are members of the same
    * highlight, and there are comment markers of either type in that.
    */
    bool canComment( int startAttr, int endAttr ) const;

    /**
    * @return 0 if highlighting which attr is a member of does not
    * define a comment region, otherwise the region is returned
    */
    signed char commentRegion(int attr) const;

    /**
     * @return the mulitiline comment start marker for the highlight
     * corresponding to @p attrib.
     */
    TQString getCommentStart( int attrib=0 ) const;

    /**
     * @return the muiltiline comment end marker for the highlight corresponding
     * to @p attrib.
     */
    TQString getCommentEnd( int attrib=0 ) const;

    /**
     * @return the single comment marker for the highlight corresponding
     * to @p attrib.
     */
    TQString getCommentSingleLineStart( int attrib=0 ) const;


    /**
     * This enum is used for storing the information where a single line comment marker should be inserted
     */
    enum CSLPos { CSLPosColumn0=0,CSLPosAfterWhitespace=1};

    /**
     * @return the single comment marker position for the highlight corresponding
     * to @p attrib.
     */
    CSLPos getCommentSingleLinePosition( int attrib=0 ) const;

    /**
    * @return the attribute for @p context.
    */
    int attribute( int context ) const;

    /**
     * map attribute to its highlighting file.
     * the returned string is used as key for m_additionalData.
     */
    TQString hlKeyForAttrib( int attrib ) const;


    void clearAttributeArrays ();

    TQMemArray<KateAttribute> *attributes (uint schema);

    inline bool noHighlighting () const { return noHl; };

    // be carefull: all documents hl should be invalidated after calling this method!
    void dropDynamicContexts();

    TQString indentation () { return m_indentation; }

  private:
    // make this private, nobody should play with the internal data pointers
    void getKateHlItemDataList(uint schema, KateHlItemDataList &);

    void init();
    void done();
    void makeContextList ();
    int makeDynamicContext(KateHlContext *model, const TQStringList *args);
    void handleKateHlIncludeRules ();
    void handleKateHlIncludeRulesRecursive(KateHlIncludeRules::iterator it, KateHlIncludeRules *list);
    int addToContextList(const TQString &ident, int ctx0);
    void addToKateHlItemDataList();
    void createKateHlItemData (KateHlItemDataList &list);
    void readGlobalKeywordConfig();
    void readWordWrapConfig();
    void readCommentConfig();
    void readIndentationConfig ();
    void readFoldingConfig ();

    // manipulates the ctxs array directly ;)
    void generateContextStack(int *ctxNum, int ctx, TQMemArray<short> *ctxs, int *posPrevLine);

    KateHlItem *createKateHlItem(KateSyntaxContextData *data, KateHlItemDataList &iDl, TQStringList *RegionList, TQStringList *ContextList);
    int lookupAttrName(const TQString& name, KateHlItemDataList &iDl);

    void createContextNameList(TQStringList *ContextNameList, int ctx0);
    int getIdFromString(TQStringList *ContextNameList, TQString tmpLineEndContext,/*NO CONST*/ TQString &unres);

    KateHlItemDataList internalIDList;

    TQValueVector<KateHlContext*> m_contexts;
    inline KateHlContext *contextNum (uint n) { if (n < m_contexts.size()) return m_contexts[n]; return 0; }

    TQMap< TQPair<KateHlContext *, TQString>, short> dynamicCtxs;

    // make them pointers perhaps
    KateEmbeddedHlInfos embeddedHls;
    KateHlUnresolvedCtxRefs unresolvedContextReferences;
    TQStringList RegionList;
    TQStringList ContextNameList;

    bool noHl;
    bool folding;
    bool casesensitive;
    TQString weakDeliminator;
    TQString deliminator;

    TQString iName;
    TQString iNameTranslated;
    TQString iSection;
    bool iHidden;
    TQString iWildcards;
    TQString iMimetypes;
    TQString identifier;
    TQString iVersion;
    TQString iAuthor;
    TQString iLicense;
    TQString m_indentation;
    int m_priority;
    int refCount;
    int startctx, base_startctx;

    TQString errorsAndWarnings;
    TQString buildIdentifier;
    TQString buildPrefix;
    bool building;
    uint itemData0;
    uint buildContext0Offset;
    KateHlIncludeRules includeRules;
    bool m_foldingIndentationSensitive;

    TQIntDict< TQMemArray<KateAttribute> > m_attributeArrays;


    /**
     * This class holds the additional properties for one highlight
     * definition, such as comment strings, deliminators etc.
     *
     * When a highlight is added, a instance of this class is appended to
     * m_additionalData, and the current position in the attrib and context
     * arrays are stored in the indexes for look up. You can then use
     * hlKeyForAttrib or hlKeyForContext to find the relevant instance of this
     * class from m_additionalData.
     *
     * If you need to add a property to a highlight, add it here.
     */
    class HighlightPropertyBag {
      public:
        TQString singleLineCommentMarker;
        TQString multiLineCommentStart;
        TQString multiLineCommentEnd;
        TQString multiLineRegion;
        CSLPos  singleLineCommentPosition;
        TQString deliminator;
        TQString wordWrapDeliminator;
    };

    /**
     * Highlight properties for each included highlight definition.
     * The key is the identifier
     */
    TQDict<HighlightPropertyBag> m_additionalData;

    /**
     * Fast lookup of hl properties, based on attribute index
     * The key is the starting index in the attribute array for each file.
     * @see hlKeyForAttrib
     */
    TQMap<int, TQString> m_hlIndex;


    TQString extensionSource;
    TQValueList<TQRegExp> regexpExtensions;
    TQStringList plainExtensions;

  public:
    inline bool foldingIndentationSensitive () { return m_foldingIndentationSensitive; }
    inline bool allowsFolding(){return folding;}
};

class KateHlManager : public TQObject
{
  TQ_OBJECT

  private:
    KateHlManager();

  public:
    ~KateHlManager();

    static KateHlManager *self();

    inline TDEConfig *getTDEConfig() { return &m_config; };

    KateHighlighting *getHl(int n);
    int nameFind(const TQString &name);

    int detectHighlighting (class KateDocument *doc);

    int findHl(KateHighlighting *h) {return hlList.find(h);}
    TQString identifierForName(const TQString&);

    // methodes to get the default style count + names
    static uint defaultStyles();
    static TQString defaultStyleName(int n, bool translateNames = false);

    void getDefaults(uint schema, KateAttributeList &);
    void setDefaults(uint schema, KateAttributeList &);

    int highlights();
    TQString hlName(int n);
    TQString hlNameTranslated (int n);
    TQString hlSection(int n);
    bool hlHidden(int n);

    void incDynamicCtxs() { ++dynamicCtxsCount; };
    uint countDynamicCtxs() { return dynamicCtxsCount; };
    void setForceNoDCReset(bool b) { forceNoDCReset = b; };

    // be carefull: all documents hl should be invalidated after having successfully called this method!
    bool resetDynamicCtxs();

  signals:
    void changed();

  private:
    int wildcardFind(const TQString &fileName);
    int mimeFind(KateDocument *);
    int realWildcardFind(const TQString &fileName);

  private:
    friend class KateHighlighting;

    TQPtrList<KateHighlighting> hlList;
    TQDict<KateHighlighting> hlDict;

    static KateHlManager *s_self;

    TDEConfig m_config;
    TQStringList commonSuffixes;

    KateSyntaxDocument *syntax;

    uint dynamicCtxsCount;
    TQTime lastCtxsReset;
    bool forceNoDCReset;
};

class KateViewHighlightAction: public Kate::ActionMenu
{
  TQ_OBJECT

  public:
    KateViewHighlightAction(const TQString& text, TQObject* parent = 0, const char* name = 0)
       : Kate::ActionMenu(text, parent, name) { init(); };

    ~KateViewHighlightAction(){;};

    void updateMenu (Kate::Document *doc);

  private:
    void init();

    TQGuardedPtr<Kate::Document> m_doc;
    TQStringList subMenusName;
    TQStringList names;
    TQPtrList<TQPopupMenu> subMenus;

  public  slots:
    void slotAboutToShow();

  private slots:
    void setHl (int mode);
};

#endif
