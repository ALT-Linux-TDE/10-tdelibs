/* This file is part of the KDE libraries
    Copyright (C) 1997 Stefan Taferner (taferner@kde.org)
    Copyright (C) 2000 Nicolas Hadacek (hadacek@kde.org)
    Copyright (C) 2001,2002 Ellis Whitehead (ellis@kde.org)

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
#ifndef TDESTDACCEL_H
#define TDESTDACCEL_H

#include <tqstring.h>
#include <tdeshortcut.h>
#include "tdelibs_export.h"

class TQKeyEvent;
class TDEAccelActions;

/**
 * \namespace TDEStdAccel
 * Convenient methods for access to the common accelerator keys in
 * the key configuration. These are the standard keybindings that should
 * be used in all KDE applications. They will be configurable,
 * so do not hardcode the default behavior.
 *
 * If you want real configurable keybindings in your applications,
 * please checkout the class TDEAccel in tdeaccel.h
 * @see TDEAccelShortcutList
 */
namespace TDEStdAccel
{
  // Always add new std-accels to the end of this enum, never in the middle!
  /**
   * Defines the identifier of all standard accelerators.
   */
  enum StdAccel {
    AccelNone,
    // File menu
    Open, New, Close, Save,
    // The Print item
    Print,
    Quit,
    // Edit menu
    Undo, Redo, Cut, Copy, Paste, SelectAll, Deselect, DeleteWordBack,
    DeleteWordForward, Find, FindNext, FindPrev, Replace,
    // Navigation
    Home, End, Prior, Next, GotoLine, AddBookmark, ZoomIn, ZoomOut,
    Up, Back, Forward, Reload, PopupMenuContext, ShowMenubar,
    // Help menu
    Help, WhatsThis,
    // Text completion
    TextCompletion, PrevCompletion, NextCompletion, SubstringCompletion,
    RotateUp, RotateDown,

    // Tabular navigation
    TabNext,           ///< @since 3.2
    TabPrev,           ///< @since 3.2

    // Full screen mode
    FullScreen,        ///< @since 3.2

    // Text Navigation
    BackwardWord,      ///< @since 3.3
    ForwardWord,       ///< @since 3.3
    BeginningOfLine,   ///< @since 3.3
    EndOfLine,         ///< @since 3.3

    PasteSelection     ///< @since 3.4

#ifndef KDE_NO_COMPAT
    , WhatThis = WhatsThis
#endif
  };

  /**
   * Returns the keybinding for @p accel.
   * @param id the id of the accelerator
   */
  TDECORE_EXPORT const TDEShortcut& shortcut(StdAccel id);

  /**
   * Returns a unique name for the given accel.
   * @param id the id of the accelerator
   * @return the unique name of the accelerator
   */
  TDECORE_EXPORT TQString name(StdAccel id);

  /**
   * Returns a localized label for user-visible display.
   * @param id the id of the accelerator
   * @return a localized label for the accelerator
   */
  TDECORE_EXPORT TQString label(StdAccel id);

  /**
   * Returns an extended WhatsThis description for the given accelerator.
   * @param id the id of the accelerator
   * @return a localized description of the accelerator
   */
  TDECORE_EXPORT TQString whatsThis(StdAccel id);

  /**
   * Return the StdAccel id of the standard accel action which
   * uses this key sequence, or AccelNone if none of them do.
   * This is used by class KKeyChooser.
   * @param keySeq the key sequence to search
   * @return the id of the standard accelerator, or AccelNone if there
   *          is none
   */
  TDECORE_EXPORT StdAccel findStdAccel( const KKeySequence &keySeq );

  /**
   * Returns the hardcoded default shortcut for @p id.
   * This does not take into account the user's configuration.
   * @param id the id of the accelerator
   * @return the default shortcut of the accelerator
   */
  TDECORE_EXPORT TDEShortcut shortcutDefault(StdAccel id);
  /**
   * Returns the hardcoded default 3 modifier shortcut for @p id.
   * This does not take into account the user's configuration.
   * @param id the id of the accelerator
   * @return the default 3 modifier shortcut
   */
  TDECORE_EXPORT TDEShortcut shortcutDefault3(StdAccel id);
  /**
   * Returns the hardcoded default 4 modifier shortcut for @p id.
   * This does not take into account the user's configuration.
   * @param id the id of the accelerator
   * @return the default 4 modifier shortcut
   */
  TDECORE_EXPORT TDEShortcut shortcutDefault4(StdAccel id);

  /**
   * Open file. Default: Ctrl-o
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& open();

  /**
   * Create a new document (or whatever). Default: Ctrl-n
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& openNew();

  /**
   * Close current document. Default: Ctrl-w
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& close();

  /**
   * Save current document. Default: Ctrl-s
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& save();

  /**
   * Print current document. Default: Ctrl-p
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& print();

  /**
   * Quit the program. Default: Ctrl-q
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& quit();

  /**
   * Undo last operation. Default: Ctrl-z
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& undo();

  /**
   * Redo. Default: Shift-Ctrl-z
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& redo();

  /**
   * Cut selected area and store it in the clipboard. Default: Ctrl-x
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& cut();

  /**
   * Copy selected area into the clipboard. Default: Ctrl-c
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& copy();

  /**
   * Paste contents of clipboard at mouse/cursor position. Default: Ctrl-v
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& paste();

  /**
   * Paste the selection at mouse/cursor position. Default: Ctrl-Shift-Insert
   * @return the shortcut of the standard accelerator
   * @since 3.4
   */
  TDECORE_EXPORT const TDEShortcut& pasteSelection();

  /**
   * Reload. Default: Ctrl-A
   * @return the shortcut of the standard accelerator
   **/
  TDECORE_EXPORT const TDEShortcut& selectAll();

  /**
   * Delete a word back from mouse/cursor position. Default: Ctrl-Backspace
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& deleteWordBack();

  /**
   * Delete a word forward from mouse/cursor position. Default: Ctrl-Delete
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& deleteWordForward();

  /**
   * Find, search. Default: Ctrl-f
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& find();

  /**
   * Find/search next. Default: F3
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& findNext();

  /**
   * Find/search previous. Default: Shift-F3
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& findPrev();

  /**
   * Find and replace matches. Default: Ctrl-r
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& replace();

  /**
   * Zoom in. Default: Ctrl-Plus
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& zoomIn();

  /**
   * Zoom out. Default: Ctrl-Minus
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& zoomOut();

  /**
   * Toggle insert/overwrite (with visual feedback, e.g. in the statusbar). Default: Insert
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& insert();

  /**
   * Goto beginning of the document. Default: Ctrl-Home
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& home();

  /**
   * Goto end of the document. Default: Ctrl-End
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& end();

  /**
   * Goto beginning of current line. Default: Home
   * @return the shortcut of the standard accelerator
   * @since 3.3
   */
  TDECORE_EXPORT const TDEShortcut& beginningOfLine();

  /**
   * Goto end of current line. Default: End
   * @return the shortcut of the standard accelerator
   * @since 3.3
   */
  TDECORE_EXPORT const TDEShortcut& endOfLine();

  /**
   * Scroll up one page. Default: Prior
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& prior();

  /**
   * Scroll down one page. Default: Next
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& next();

  /**
   * Go to line. Default: Ctrl+G
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& gotoLine();

  /**
   * Add current page to bookmarks. Default: Ctrl+B
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& addBookmark();

  /**
   * Next Tab. Default: Ctrl-<
   * @return the shortcut of the standard accelerator
   * @since 3.2
   */
  TDECORE_EXPORT const TDEShortcut& tabNext();

  /**
   * Previous Tab. Default: Ctrl->
   * @return the shortcut of the standard accelerator
   * @since 3.2
   */
  TDECORE_EXPORT const TDEShortcut& tabPrev();

  /**
   * Full Screen Mode. Default: Ctrl+Shift+F
   * @return the shortcut of the standard accelerator
   * @since 3.2
   */
  TDECORE_EXPORT const TDEShortcut& fullScreen();

  /**
   * Help the user in the current situation. Default: F1
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& help();

  /**
   * Complete text in input widgets. Default Ctrl+E
   * @return the shortcut of the standard accelerator
   **/
  TDECORE_EXPORT const TDEShortcut& completion();

  /**
   * Iterate through a list when completion returns
   * multiple items. Default: Ctrl+Up
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& prevCompletion();

  /**
   * Iterate through a list when completion returns
   * multiple items. Default: Ctrl+Down
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& nextCompletion();

  /**
   * Find a string within another string or list of strings.
   * Default: Ctrl-T
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& substringCompletion();

  /**
   * Help users iterate through a list of entries. Default: Up
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& rotateUp();

  /**
   * Help users iterate through a list of entries. Default: Down
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& rotateDown();

  /**
   * popup a context menu. Default: Menu
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& popupMenuContext();

  /**
   * What's This button. Default: Shift+F1
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& whatsThis();

  /**
   * Reload. Default: F5
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& reload();

  /**
   * Up. Default: Alt+Up
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& up();

  /**
   * Back. Default: Alt+Left
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& back();

  /**
   * Forward. Default: ALT+Right
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& forward();

  /**
   * BackwardWord. Default: Ctrl+Left
   * @return the shortcut of the standard accelerator
   * @since 3.3
   */
  TDECORE_EXPORT const TDEShortcut& backwardWord();

  /**
   * ForwardWord. Default: Ctrl+Right
   * @return the shortcut of the standard accelerator
   * @since 3.3
   */
  TDECORE_EXPORT const TDEShortcut& forwardWord();

  /**
   * Show Menu Bar.  Default: Ctrl-M
   * @return the shortcut of the standard accelerator
   */
  TDECORE_EXPORT const TDEShortcut& showMenubar();

#if !defined(KDE_NO_COMPAT) && !defined(__KSTDACCEL_CPP_)
  /**
   * @deprecated
   * Obsolete.  Use name().  Returns a string representation for @p accel.
   */
  TDECORE_EXPORT TQString action(StdAccel id) TDE_DEPRECATED;
  /**
   * @deprecated
   * Obsolete.  Use desc().  Returns a localized description of @p accel.
   */
  TDECORE_EXPORT TQString description(StdAccel id) TDE_DEPRECATED;
  /**
   * @deprecated
   * Obsolete.  Use shortcut().  Returns the keybinding for @p accel.
   */
  TDECORE_EXPORT int key(StdAccel) TDE_DEPRECATED;
  /**
   * @deprecated
   * Obsolete.  Use shortcutDefault().
   */
  TDECORE_EXPORT int defaultKey(StdAccel accel) TDE_DEPRECATED;

  /**
   * @deprecated.  Use KKey(const TQKeyEvent*) == KKey(int).
   *
   * Compare the keys generated by the key event with
   * the value of the integer.
   *
   * If a modifier (Shift, Alt, Ctrl) key is present in
   * TQKeyEvent, its sum with the actual key value
   * is used for comparing it with the integer parameter.
   *
   * @param pEvent the key event to be used in the comparison.
   * @param keyQt the int value to be compared to the key event.
   *
   * @return true if the int value matches the integer representation of the QKeyEvent
   */
  TDECORE_EXPORT bool isEqual(const TQKeyEvent* pEvent, int keyQt) TDE_DEPRECATED;
#endif // !KDE_NO_COMPAT

}

#endif
