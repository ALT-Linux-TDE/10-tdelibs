/* This file is part of the KDE libraries
   Copyright (C) 1999,2000 Kurt Granroth <granroth@kde.org>

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

#ifndef _KSTDACTION_PRIVATE_H_
#define _KSTDACTION_PRIVATE_H_

#include <tdelocale.h>
#include <tdestdaccel.h>

namespace KStdAction
{

struct KStdActionInfo
{
	StdAction id;
	TDEStdAccel::StdAccel idAccel;
	const char* psName;
	const char* psLabel;
	const char* psWhatsThis;
	const char* psIconName;
};

static const KStdActionInfo g_rgActionInfo[] =
{
	{ New,           TDEStdAccel::New, "file_new", I18N_NOOP("&New"), 0, "document-new" },
	{ Open,          TDEStdAccel::Open, "file_open", I18N_NOOP("&Open..."), 0, "document-open" },
	{ OpenRecent,    TDEStdAccel::AccelNone, "file_open_recent", I18N_NOOP("Open &Recent"), 0, "document-open" },
	{ Save,          TDEStdAccel::Save, "file_save", I18N_NOOP("&Save"), 0, "document-save" },
	{ SaveAs,        TDEStdAccel::AccelNone, "file_save_as", I18N_NOOP("Save &As..."), 0, "document-save-as" },
	{ Revert,        TDEStdAccel::AccelNone, "file_revert", I18N_NOOP("Re&vert"), 0, "revert" },
	{ Close,         TDEStdAccel::Close, "file_close", I18N_NOOP("&Close"), 0, "window-close" },
	{ Print,         TDEStdAccel::Print, "file_print", I18N_NOOP("&Print..."), 0, "document-print" },
	{ PrintPreview,  TDEStdAccel::AccelNone, "file_print_preview", I18N_NOOP("Print Previe&w..."), 0, "document-print-preview" },
	{ Mail,          TDEStdAccel::AccelNone, "file_mail", I18N_NOOP("&Mail..."), 0, "mail-send" },
	{ Quit,          TDEStdAccel::Quit, "file_quit", I18N_NOOP("&Quit"), 0, "application-exit" },

	{ Undo,          TDEStdAccel::Undo, "edit_undo", I18N_NOOP("&Undo"), 0, "edit-undo" },
	{ Redo,          TDEStdAccel::Redo, "edit_redo", I18N_NOOP("Re&do"), 0, "edit-redo" },
	{ Cut,           TDEStdAccel::Cut, "edit_cut", I18N_NOOP("Cu&t"), 0, "edit-cut" },
	{ Copy,          TDEStdAccel::Copy, "edit_copy", I18N_NOOP("&Copy"), 0, "edit-copy" },
	{ Paste,         TDEStdAccel::Paste, "edit_paste", I18N_NOOP("&Paste"), 0, "edit-paste" },
	{ PasteText,     TDEStdAccel::Paste, "edit_paste", I18N_NOOP("&Paste"), 0, "edit-paste" },
	{ Clear,         TDEStdAccel::AccelNone, "edit_clear", I18N_NOOP("C&lear"), 0, "edit-clear" },
	{ SelectAll,     TDEStdAccel::SelectAll, "edit_select_all", I18N_NOOP("Select &All"), 0, 0 },
	{ Deselect,      TDEStdAccel::Deselect, "edit_deselect", I18N_NOOP("Dese&lect"), 0, 0 },
	{ Find,          TDEStdAccel::Find, "edit_find", I18N_NOOP("&Find..."), 0, "edit-find" },
	{ FindNext,      TDEStdAccel::FindNext, "edit_find_next", I18N_NOOP("Find &Next"), 0, "go-next" },
	// FIXME: rename edit_find_last to edit_find_prev for KDE 4
	{ FindPrev,      TDEStdAccel::FindPrev, "edit_find_last", I18N_NOOP("Find Pre&vious"), 0, "go-previous" },
	{ Replace,       TDEStdAccel::Replace, "edit_replace", I18N_NOOP("&Replace..."), 0, 0 },

	{ ActualSize,    TDEStdAccel::AccelNone, "view_actual_size", I18N_NOOP("&Actual Size"), 0, "zoom-original" },
	{ FitToPage,     TDEStdAccel::AccelNone, "view_fit_to_page", I18N_NOOP("&Fit to Page"), 0, "view_fit_window" },
	{ FitToWidth,    TDEStdAccel::AccelNone, "view_fit_to_width", I18N_NOOP("Fit to Page &Width"), 0, "view_fit_width" },
	{ FitToHeight,   TDEStdAccel::AccelNone, "view_fit_to_height", I18N_NOOP("Fit to Page &Height"), 0, "view_fit_height" },
	{ ZoomIn,        TDEStdAccel::ZoomIn, "view_zoom_in", I18N_NOOP("Zoom &In"), 0, "zoom-in" },
	{ ZoomOut,       TDEStdAccel::ZoomOut, "view_zoom_out", I18N_NOOP("Zoom &Out"), 0, "zoom-out" },
	{ Zoom,          TDEStdAccel::AccelNone, "view_zoom", I18N_NOOP("&Zoom..."), 0, "viewmag" },
        // KDE4: give Redisplay the shortcut TDEStdAccel::AccelReload
	{ Redisplay,     TDEStdAccel::AccelNone, "view_redisplay", I18N_NOOP("&Redisplay"), 0, "reload" },

	{ Up,            TDEStdAccel::Up, "go_up", I18N_NOOP("&Up"), 0, "go-up" },
	// The following three have special i18n() needs for sLabel
	{ Back,          TDEStdAccel::Back, "go_back", 0, 0, "back" },
	{ Forward,       TDEStdAccel::Forward, "go_forward", 0, 0, "forward" },
	{ Home,          TDEStdAccel::Home, "go_home", 0, 0, "go-home" },
	{ Prior,         TDEStdAccel::Prior, "go_previous", I18N_NOOP("&Previous Page"), 0, "back" },
	{ Next,          TDEStdAccel::Next, "go_next", I18N_NOOP("&Next Page"), 0, "forward" },
	{ Goto,          TDEStdAccel::AccelNone, "go_goto", I18N_NOOP("&Go To..."), 0, 0 },
	{ GotoPage,      TDEStdAccel::AccelNone, "go_goto_page", I18N_NOOP("&Go to Page..."), 0, "goto" },
	{ GotoLine,      TDEStdAccel::GotoLine, "go_goto_line", I18N_NOOP("&Go to Line..."), 0, 0 },
	{ FirstPage,     TDEStdAccel::Home, "go_first", I18N_NOOP("&First Page"), 0, "go-first" },
	{ LastPage,      TDEStdAccel::End, "go_last", I18N_NOOP("&Last Page"), 0, "go-last" },

	{ AddBookmark,   TDEStdAccel::AddBookmark, "bookmark_add", I18N_NOOP("&Add Bookmark"), 0, "bookmark_add" },
	{ EditBookmarks, TDEStdAccel::AccelNone, "bookmark_edit", I18N_NOOP("&Edit Bookmarks"), 0, "bookmark" },

	{ Spelling,      TDEStdAccel::AccelNone, "tools_spelling", I18N_NOOP("&Spelling..."), 0, "tools-check-spelling" },

	{ ShowMenubar,   TDEStdAccel::ShowMenubar, "options_show_menubar", I18N_NOOP("Show &Menubar"), 0, "showmenu" },
	{ ShowToolbar,   TDEStdAccel::AccelNone, "options_show_toolbar", I18N_NOOP("Show &Toolbar"), 0, 0 },
	{ ShowStatusbar, TDEStdAccel::AccelNone, "options_show_statusbar", I18N_NOOP("Show St&atusbar"), 0, 0 },
	{ FullScreen,    TDEStdAccel::FullScreen, "fullscreen", I18N_NOOP("F&ull Screen Mode"), 0, "view-fullscreen" },
	{ SaveOptions,   TDEStdAccel::AccelNone, "options_save_options", I18N_NOOP("&Save Settings"), 0, 0 },
	{ KeyBindings,   TDEStdAccel::AccelNone, "options_configure_keybinding", I18N_NOOP("Configure S&hortcuts..."), 0,"configure_shortcuts" },
	{ Preferences,   TDEStdAccel::AccelNone, "options_configure", I18N_NOOP("&Configure %1..."), 0, "configure" },
	{ ConfigureToolbars, TDEStdAccel::AccelNone, "options_configure_toolbars", I18N_NOOP("Configure Tool&bars..."), 0,"configure_toolbars" },
	{ ConfigureNotifications, TDEStdAccel::AccelNone, "options_configure_notifications", I18N_NOOP("Configure &Notifications..."), 0, "knotify" },

	// the idea here is that Contents is used in menus, and Help in dialogs, so both share the same
	// shortcut
	{ Help,          TDEStdAccel::Help, "help", 0, 0, "help" },
	{ HelpContents,  TDEStdAccel::Help, "help_contents", I18N_NOOP("%1 &Handbook"), 0, "contents" },
	{ WhatsThis,     TDEStdAccel::WhatsThis, "help_whats_this", I18N_NOOP("What's &This?"), 0, "contexthelp" },
	{ TipofDay,      TDEStdAccel::AccelNone, "help_show_tip", I18N_NOOP("Tip of the &Day"), 0, "idea" },
	{ ReportBug,     TDEStdAccel::AccelNone, "help_report_bug", I18N_NOOP("&Report Bug/Request Enhancement..."), 0, "bug" },
	{ SwitchApplicationLanguage,     TDEStdAccel::AccelNone, "switch_application_language", I18N_NOOP("Switch application &language..."), 0, "preferences-desktop-locale" },
	{ AboutApp,      TDEStdAccel::AccelNone, "help_about_app", I18N_NOOP("&About %1"), 0, 0 },
	{ AboutKDE,      TDEStdAccel::AccelNone, "help_about_kde", I18N_NOOP("About &TDE"), 0,"about_kde" },
	{ ActionNone, TDEStdAccel::AccelNone, 0, 0, 0, 0 }
};

inline const KStdActionInfo* infoPtr( StdAction id )
{
	for( uint i = 0; g_rgActionInfo[i].id != ActionNone; i++ ) {
		if( g_rgActionInfo[i].id == id )
			return &g_rgActionInfo[i];
	}
	return 0;
}

static inline TQStringList internal_stdNames()
{
    TQStringList result;

    for( uint i = 0; g_rgActionInfo[i].id != ActionNone; i++ )
        if (g_rgActionInfo[i].psLabel)
            result.append(i18n(g_rgActionInfo[i].psLabel));
    return result;
}

}

#endif
