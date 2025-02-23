/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>
   Copyright (C) 2014 Timothy Pearson <kb9vqf@pearsoncomputing.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef _KBUGREPORT_H__
#define _KBUGREPORT_H__

#include <kdialogbase.h>

class TQMultiLineEdit;
class TQLineEdit;
class TQHButtonGroup;
class TDEProcess;
class TDEAboutData;
class KBugReportPrivate;

/**
 * @short A dialog box for sending bug reports.
 *
 * All the information needed by the dialog box
 * (program name, version, bug-report address, etc.)
 * comes from the TDEAboutData class.
 * Make sure you create an instance of TDEAboutData and pass it
 * to TDECmdLineArgs.
 *
 * @author David Faure <faure@kde.org>
 */
class TDEUI_EXPORT KBugReport : public KDialogBase
{
  TQ_OBJECT
public:
  /**
   * Creates a bug-report dialog.
   * Note that you shouldn't have to do this manually,
   * since KHelpMenu takes care of the menu item
   * for "Report Bug..." and of creating a KBugReport dialog.
   */
  KBugReport( TQWidget * parent = 0L, bool modal=true, const TDEAboutData *aboutData = 0L );
  /**
   * Destructor
   */
  virtual ~KBugReport();

  TQString compilerVersion();
  TQString operatingSystem();
  TQString tdeVersion();

protected slots:
  /**
   * "Configure email" has been clicked - this calls tdecmshell System/email
   */
  virtual void slotConfigureEmail();
  /**
   * Sets the "From" field from the e-mail configuration
   * Called at creation time, but also after "Configure email" is closed.
   */
  virtual void slotSetFrom();
  /**
   * The URL-Label "http://bugs.trinitydesktop.org/" was clicked.
   * @deprecated remove in KDE4.0
   */
  virtual void slotUrlClicked(const TQString &);
  /**
   * OK has been clicked
   */
  virtual void slotOk( void );
  /**
   * Cancel has been clicked
   */
  virtual void slotCancel();

  /**
   * Application combo selection changed (and was activated)
   */
  void appChanged(int);
  /**
   * Update the url to match the current os, compiler, selected app, etc
   */
  void updateURL();

protected:
  /**
   * A complete copy of the bug report
   * @return TQString copy of the bug report.
   */
  TQString text() const;
  /**
   * Attempt to e-mail the bug report.
   * @return true on success
   */
  bool sendBugReport();

  TDEProcess * m_process;
  const TDEAboutData * m_aboutData;

  TQMultiLineEdit * m_lineedit;
  TQLineEdit * m_subject;
  TQLabel * m_from;
  TQLabel * m_version;
  TQString m_strVersion;
  TQHButtonGroup * m_bgSeverity;
  TQPushButton * m_configureEmail;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KBugReportPrivate *d;
};

#endif

