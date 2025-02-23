/* This file is part of the KDE libraries
   Copyright (C) 2001-2003 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002, 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>

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

#ifndef __KATE_SCHEMA_H__
#define __KATE_SCHEMA_H__

#include "katehighlight.h"
#include "katedialogs.h"

#include <tqstringlist.h>
#include <tqintdict.h>
#include <tqmap.h>
#include <tqlistview.h>
#include <tqfont.h>

#include <tdeconfig.h>
#include <tdeaction.h>

class KateView;
class KateStyleListItem;
class KateStyleListCaption;

class KColorButton;

class TQPopupMenu;
class KComboBox;

class KateSchemaManager
{
  public:
    KateSchemaManager ();
    ~KateSchemaManager ();

    /**
     * Schema Config changed, update all renderers
     */
    void update (bool readfromfile = true);

    /**
     * return tdeconfig with right group set or set to Normal if not there
     */
    TDEConfig *schema (uint number);

    void addSchema (const TQString &t);

    void removeSchema (uint number);

    /**
     * is this schema valid ? (does it exist ?)
     */
    bool validSchema (uint number);

    /**
     * if not found, defaults to 0
     */
    uint number (const TQString &name);

    /**
     * group names in the end, no i18n involved
     */
    TQString name (uint number);

    /**
     * Don't modify, list with the names of the schemas (i18n name for the default ones)
     */
    const TQStringList &list () { return m_schemas; }

    static TQString normalSchema ();
    static TQString printingSchema ();

  private:
    TDEConfig m_config;
    TQStringList m_schemas;
};


class KateViewSchemaAction : public TDEActionMenu
{
  TQ_OBJECT

  public:
    KateViewSchemaAction(const TQString& text, TQObject* parent = 0, const char* name = 0)
       : TDEActionMenu(text, parent, name) { init(); };

    ~KateViewSchemaAction(){;};

    void updateMenu (KateView *view);

  private:
    void init();

    TQGuardedPtr<KateView> m_view;
    TQStringList names;
    int last;

  public  slots:
    void slotAboutToShow();

  private slots:
    void setSchema (int mode);
};

//
// DIALOGS
//

/*
    TQListView that automatically adds columns for KateStyleListItems and provides a
    popup menu and a slot to edit a style using the keyboard.
    Added by anders, jan 23 2002.
*/
class KateStyleListView : public TQListView
{
  TQ_OBJECT

  friend class KateStyleListItem;

  public:
    KateStyleListView( TQWidget *parent=0, bool showUseDefaults=false);
    ~KateStyleListView() {};
    /* Display a popupmenu for item i at the specified global position, eventually with a title,
       promoting the context name of that item */
    void showPopupMenu( KateStyleListItem *i, const TQPoint &globalPos, bool showtitle=false );
    void emitChanged() { emit changed(); };

    void setBgCol( const TQColor &c ) { bgcol = c; }
    void setSelCol( const TQColor &c ) { selcol = c; }
    void setNormalCol( const TQColor &c ) { normalcol = c; }

  private slots:
    /* Display a popupmenu for item i at item position */
    void showPopupMenu( TQListViewItem *i, const TQPoint &globalPos );
    /* call item to change a property, or display a menu */
    void slotMousePressed( int, TQListViewItem*, const TQPoint&, int );
    /* asks item to change the property in q */
    void mSlotPopupHandler( int z );
    void unsetColor( int );

  signals:
    void changed();

  private:
    TQColor bgcol, selcol, normalcol;
    TQFont docfont;
};

class KateSchemaConfigColorTab : public TQWidget
{
  TQ_OBJECT

  public:
    KateSchemaConfigColorTab( TQWidget *parent = 0, const char *name = 0 );
    ~KateSchemaConfigColorTab();

  private:
    KColorButton *m_back;
    KColorButton *m_selected;
    KColorButton *m_current;
    KColorButton *m_bracket;
    KColorButton *m_wwmarker;
    KColorButton *m_iconborder;
    KColorButton *m_tmarker;
    KColorButton *m_linenumber;

    KColorButton *m_markers;           // bg color for current selected marker
    KComboBox* m_combobox;             // switch marker type

    // Class for storing the properties on 1 schema.
    class SchemaColors {
      public:
        TQColor back, selected, current, bracket, wwmarker, iconborder, tmarker, linenumber;
        TQMap<int, TQColor> markerColors;  // stores all markerColors
    };

    // schemaid=data, created when a schema is entered
    TQMap<int,SchemaColors> m_schemas;
    // current schema
    int m_schema;

  public slots:
    void apply();
    void schemaChanged( int newSchema );

  signals:
    void changed(); // connected to parentWidget()->parentWidget() TQ_SLOT(slotChanged)

  protected slots:
    void slotMarkerColorChanged(const TQColor&);
    void slotComboBoxChanged(int index);
};

typedef TQMap<int,TQFont> FontMap; // ### remove it

class KateSchemaConfigFontTab : public TQWidget
{
  TQ_OBJECT

  public:
    KateSchemaConfigFontTab( TQWidget *parent = 0, const char *name = 0 );
    ~KateSchemaConfigFontTab();

  public:
    void readConfig (TDEConfig *config);

  public slots:
    void apply();
    void schemaChanged( int newSchema );

  signals:
    void changed(); // connected to parentWidget()->parentWidget() TQ_SLOT(slotChanged)

  private:
    class TDEFontChooser *m_fontchooser;
    FontMap m_fonts;
    int m_schema;

  private slots:
    void slotFontSelected( const TQFont &font );
};

class KateSchemaConfigFontColorTab : public TQWidget
{
  TQ_OBJECT

  public:
    KateSchemaConfigFontColorTab( TQWidget *parent = 0, const char *name = 0 );
    ~KateSchemaConfigFontColorTab();

  public:
    void schemaChanged (uint schema);
    void reload ();
    void apply ();

    KateAttributeList *attributeList (uint schema);

  private:
    KateStyleListView *m_defaultStyles;
    TQIntDict<KateAttributeList> m_defaultStyleLists;
};

class KateSchemaConfigHighlightTab : public TQWidget
{
  TQ_OBJECT

  public:
    KateSchemaConfigHighlightTab( TQWidget *parent = 0, const char *name = 0, KateSchemaConfigFontColorTab *page = 0, uint hl = 0 );
    ~KateSchemaConfigHighlightTab();

  public:
    void schemaChanged (uint schema);
    void reload ();
    void apply ();

  protected slots:
    void hlChanged(int z);

  private:
    KateSchemaConfigFontColorTab *m_defaults;

    TQComboBox *hlCombo;
    KateStyleListView *m_styles;

    uint m_schema;
    int m_hl;

    TQIntDict< TQIntDict<KateHlItemDataList> > m_hlDict;
};

class KateSchemaConfigPage : public KateConfigPage
{
  TQ_OBJECT

  public:
    KateSchemaConfigPage ( TQWidget *parent, class KateDocument *doc=0 );
    ~KateSchemaConfigPage ();

  public slots:
    void apply();
    void reload();
    void reset();
    void defaults();

  private slots:
    void update ();
    void deleteSchema ();
    void newSchema ();
    void schemaChanged (int schema);

    void newCurrentPage (TQWidget *w);

  private:
    int m_lastSchema;
    int m_defaultSchema;

    class TQTabWidget *m_tabWidget;
    class TQPushButton *btndel;
    class TQComboBox *defaultSchemaCombo;
    class TQComboBox *schemaCombo;
    KateSchemaConfigColorTab *m_colorTab;
    KateSchemaConfigFontTab *m_fontTab;
    KateSchemaConfigFontColorTab *m_fontColorTab;
    KateSchemaConfigHighlightTab *m_highlightTab;
};

#endif
