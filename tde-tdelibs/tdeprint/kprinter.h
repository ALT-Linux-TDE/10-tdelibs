/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef KPRINTER_H
#define KPRINTER_H

#include <tqpaintdevice.h>
#include <tqprinter.h>
#include <tqmap.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqvaluelist.h>
#include <tdemacros.h>
#include <tdelibs_export.h>

#include <tdeprint/kpreloadobject.h>

class KPrinterImpl;
class KPrintDialogPage;
class KPrinterPrivate;
class DrPageSize;

/**
 * This class is the main interface to access the TDE print framework. It allows TDE
 * applications to easily access the print system, through an interface that is compatible
 * with TQPrinter. So that the porting of an existing application should be as easy as
 * replacing any occurrence of TQPrinter by KPrinter. However applications that explicitly
 * used the TQPrintDialog class must be changed to the standard KPrinter way of accessing
 * the print dialog.
 *
 * Basically, a KDE application will use the KPrinter object as a paint device, and will draw
 * on it using TQPainter. In a standard application, the use of the KPrinter class will
 * look like this:
 * \code
 * #include <kprinter.h>
 * #include <tqpainter.h>
 *
 * void MyClass::doPrint()
 * {
 *   KPrinter printer;
 *
 *   // do some printer initialization
 *   printer.setFullPage( true );
 *
 *   // initialize the printer using the print dialog
 *   if ( printer.setup( this ) )
 *   {
 *     // create a painter to paint on the printer object
 *     TQPainter painter;
 *
 *     // start painting
 *     painter.begin( &printer );
 *     <do something>
 *     // stop painting, this will automatically send the print data to the printer
 *     painter.end();
 *   }
 * }
 * \endcode
 *
 * The KPrinter interface also allows some more advanced possibilities, like the customization
 * of the print dialog to integrate application specific print options. This is done by subclassing
 * KPrintDialogPage and adding the page using addDialogPage().
 *
 * When compiling an application that uses KPrinter, you must link to -ltdeprint, which when
 * using the standard KDE build framework can be done by adding $(LIB_TDEPRINT) to _LDADD.
 *
 * @short The central interface class to access the TDE print system.
 * @author Michael Goffioul
 * @see TQPrinter, KPrintDialogPage
 */
class TDEPRINT_EXPORT KPrinter : public TQPaintDevice, public KPReloadObject
{
friend class KPrinterWrapper;
friend class KPrinterImpl;
public:
	// Print global settings (set via static functions)
	/**
	 * Defines the standard pages available for the print dialog:
	 *  @li @p CopiesPage: page and copies selection (included by default)
	 *  @li @p FilesPage: file selection (only used by kprinter utility)
	 *  @li @p Custom: unused
	 *
	 * @see addStandardPage(), removeStandardPage()
	 */
	enum StandardPageType { CopiesPage = 0x01, FilesPage = 0x02, Custom = 0x10 };
	/**
	 * Defines whether the application can perform page selection itself or not.
	 * Some print systems (like CUPS) can do page selection, in this case the
	 * page selection will be enabled in the print dialog, otherwise it will be
	 * disabled. However some applications (word processor) can do page selection
	 * themselves, then it will be enabled in the print dialog, whatever the
	 * print system used. But such an application has to tell tdeprint about its
	 * feature:
	 *  @li @p ApplicationSide: the application will do page selection
	 *  @li @p SystemSide: page selection will be left to the print system, if available (default)
	 *
	 * @see setPageSelection(), pageSelection()
	 */
	enum PageSelectionType { ApplicationSide = 0x01, SystemSide = 0x02 };
	/**
	 * Defines the type of the application, this affects the GUI of the print dialog:
	 *  @li @p Dialog: print dialog used in an application (default)
	 *  @li @p StandAlone: print dialog used as a standalone widget
	 *  @li @p StandAlonePersistent: print dialog used as standalone widget, but persistent (do not use)
	 *
	 * @internal
	 * @see setApplicationType(), applicationType()
	 */
	enum ApplicationType { Dialog = 0x00, StandAlone = 0x01, StandAlonePersistent = 0x02 };

	// TQPrinter extension
	/**
	 * Defines the page set to print:
	 *  @li @p AllPages: all pages
	 *  @li @p OddPages: only odd pages
	 *  @li @p EvenPages: only even pages
	 *
	 * @see pageSet()
	 */
	enum PageSetType { AllPages = 0x00, OddPages = 0x01, EvenPages = 0x02 };
	/**
	 * Defines the collate property of the printer (if supported by the print system):
	 *  @li @p Collate: copies collated (1-2-3-..., 1-2-3-...)
	 *  @li @p Uncollate: copies uncollated (1-1-..., 2-2-..., 3-3-...)
	 *
	 * @see setCollate(), collate()
	 */
	enum CollateType { Collate = 0x00, Uncollate = 0x01 };

	// from TQPrinter class
	/**
	 * Defines the color mode of the printer
	 * @see TQPrinter::ColorMode
	 */
	enum ColorMode { GrayScale = TQPrinter::GrayScale, Color = TQPrinter::Color };
	/**
	 * Defines the orientation of the paper
	 * @see TQPrinter::Orientation
	 */
	enum Orientation { Portrait = TQPrinter::Portrait, Landscape = TQPrinter::Landscape };
	/**
	 * Defines the page order of the print job
	 * @see TQPrinter::PageOrder
	 */
	enum PageOrder { FirstPageFirst = TQPrinter::FirstPageFirst, LastPageFirst = TQPrinter::LastPageFirst };
	/**
	 * Defines the paper size to use
	 * @see TQPrinter::PageSize
	 */
	enum PageSize
	{
		A4 = TQPrinter::A4,
		B5 = TQPrinter::B5,
		Letter = TQPrinter::Letter,
		Legal = TQPrinter::Legal,
		Executive = TQPrinter::Executive,
		A0 = TQPrinter::A0,
		A1 = TQPrinter::A1,
		A2 = TQPrinter::A2,
		A3 = TQPrinter::A3,
		A5 = TQPrinter::A5,
		A6 = TQPrinter::A6,
		A7 = TQPrinter::A7,
		A8 = TQPrinter::A8,
		A9 = TQPrinter::A9,
		B0 = TQPrinter::B0,
		B1 = TQPrinter::B1,
		B10 = TQPrinter::B10,
		B2 = TQPrinter::B2,
		B3 = TQPrinter::B3,
		B4 = TQPrinter::B4,
		B6 = TQPrinter::B6,
		B7 = TQPrinter::B7,
		B8 = TQPrinter::B8,
		B9 = TQPrinter::B9,
		C5E = TQPrinter::C5E,
		Comm10E = TQPrinter::Comm10E,
		DLE = TQPrinter::DLE,
		Folio = TQPrinter::Folio,
		Ledger = TQPrinter::Ledger,
		Tabloid = TQPrinter::Tabloid,
		NPageSize = TQPrinter::NPageSize
	};

	// constructors / destructor
	/**
	 * Constructor. This also restores/saves the state from a previous KPrinter object created within
	 * the same application run, if @p restore is true. Setting @p restore to false may
	 * be useful if you want an independent/clean KPrinter object.
	 * @param restore if true, options will be restored/saved between successive KPrinter objects
	 * @param m the mode to establish the KPrinter in (see TQPrinter::PrinterMode)
	 */
	KPrinter(bool restore = true, TQPrinter::PrinterMode m = TQPrinter::ScreenResolution );
	/**
	 * Destructor. This also saves the current KPrinter state for future printing.
	 */
	~KPrinter();

	// TQPrinter interface (+ some extensions)
	/**
	 * See TQPrinter::newPage().
	 */
	bool newPage();
	/**
	 * See TQPrinter::abort().
	 */
	bool abort();
	/**
	 * See TQPrinter::aborted(.)
	 */
	bool aborted() const;
	/**
	 * See TQPrinter::outputToFile().
	 */
	bool outputToFile() const;
	/**
	 * See TQPrinter::setOutputToFile().
	 */
	void setOutputToFile(bool);
	/**
	 * See TQPrinter::outputFileName().
	 */
	TQString outputFileName() const;
	/**
	 * See TQPrinter::setOutputFileName().
	 */
	void setOutputFileName(const TQString&);
	/**
	 * See TQPrinter::docName().
	 */
	TQString docName() const;
	/**
	 * See TQPrinter::setDocName().
	 */
	void setDocName(const TQString&);
	/**
	 * See TQPrinter::creator().
	 */
	TQString creator() const;
	/**
	 * See TQPrinter::setCreator().
	 */
	void setCreator(const TQString&);
	/**
	 * See TQPrinter::fullPage().
	 */
	bool fullPage() const;
	/**
	 * See TQPrinter::setFullPage().
	 */
	void setFullPage(bool);
	/**
	 * See TQPrinter::colorMode().
	 */
	ColorMode colorMode() const;
	/**
	 * See TQPrinter::setColorMode().
	 */
	void setColorMode(ColorMode);
	/**
	 * See TQPrinter::numCopies().
	 */
	int numCopies() const;
	/**
	 * See TQPrinter::setNumCopies().
	 */
	void setNumCopies(int n);
	/**
	 * See TQPrinter::orientation().
	 */
	Orientation orientation() const;
	/**
	 * See TQPrinter::setOrientation().
	 */
	void setOrientation(Orientation, bool locking = false);
	/**
	 * See TQPrinter::pageOrder().
	 */
	PageOrder pageOrder() const;
	/**
	 * See TQPrinter::setPageOrder().
	 */
	void setPageOrder(PageOrder);
	/**
	 * Returns the collate status of the current KPrinter.
	 */
	CollateType collate() const;
	/**
	 * Sets the collate status for the current KPrinter to @p type.
	 */
	void setCollate(CollateType type);
	/**
	 * See TQPrinter::minPage().
	 */
	int minPage() const;
	/**
	 * See TQPrinter::maxPage().
	 */
	int maxPage() const;
	/**
	 * See TQPrinter::setMinMax().
	 */
	void setMinMax(int, int);
	/**
	 * Returns the first page to be printed.
         * @deprecated Applications
	 * should use pageList() instead, which takes into account all options: collate,
	 * page order, ranges, page set, ...
	 *
	 * @see pageList()
	 */
	int fromPage() const TDE_DEPRECATED;
	/**
	 * Returns the last page to be printed.
         * @deprecated Applications
	 * should use pageList() instead, which takes into account all options: collate,
	 * page order, ranges, page set, ...
	 *
	 * @see pageList()
	 */
	int toPage() const;
	/**
	 * Sets the first and last page to be printed. See TQPrinter::setFromTo().
	 */
	void setFromTo(int, int);
	/**
	 * See TQPrinter::pageSize().
	 */
	PageSize pageSize() const;
	/**
	 * See TQPrinter::setPageSize().
	 */
	void setPageSize(PageSize, bool locking = false);
	/**
	 * See TQPrinter::printerName().
	 */
	TQString printerName() const;
	/**
	 * See TQPrinter::setPrinterName().
	 */
	void setPrinterName(const TQString&);
	/**
	 * Returns the print program as set by setPrintProgram() or by the print dialog
	 * if a special printer has been selected.
	 * @return the print command line
	 * @see setPrintProgram()
	 */
	TQString printProgram() const;
	/**
	 * Sets the command line to use when printing. This function is useful
	 * when using a KPrinter object without the print dialog, to control what
	 * to print and how to do it. The command line may contain the following
	 * tags:
	 * @li %in : the input file to the print program. It is required and automatically
	 *           appended at the end of the command line if not present.
	 * @li %out : the output file. Use this tag in conjunction with setOutputToFile()
	 *            and setOutputFileName()
	 * @li %psl : the page size in lower case. This may be useful with some
	 *            programs like gs.
	 * \code
	 * void createPNGOutputFile(const TQString& filename)
	 * {
	 *   // use a clean KPrinter object
	 *   KPrinter prt(false);
	 *
	 *   prt.setOutputToFile( true );
	 *   prt.setOutputFileName( filename );
	 *   prt.setPrintProgram( "gs -sDEVICE=png256 -sPAPERSIZE=%psl -sOutputFile=%out %in" );
	 *
	 *   TQPainter painter( &prt );
	 *   doPaint( &painter );
	 * }
	 * \endcode
	 * @param cmd the print command line (containing at least the @p %in tag)
	 * @see printProgram()
	 */
	void setPrintProgram(const TQString& cmd);
	/**
	 * See TQPrinter::printerSelectionOption(). Unused.
	 */
	TQString printerSelectionOption() const;
	/**
	 * See TQPrinter::setPrinterSelectionOption(). Unused
	 */
	void setPrinterSelectionOption(const TQString&);
	/**
	 * Returns the current page number.
	 * @see setCurrentPage()
	 */
	int currentPage() const;
	/**
	 * Sets the current page number. This page number will be used if the user
	 * selected "current page" in the print dialog. This option will only be
	 * enabled if the application does page selection itself and the application
	 * has called setCurrentPage() before opening the print dialog:
	 * \code
	 * MyClass::MyClass()
	 * {
	 *   // declares my application able to do page selection
	 *   KPrinter::setPageSelection( KPrinter::ApplicationSide );
	 * }
	 *
	 * void MyClass::doPrint()
	 * {
	 *   KPrinter printer;
	 *
	 *   printer.setCurrentPage( mycurrentpage );
	 *   if ( printer.setup( this ) )
	 *   {
	 *     TQValueList<int> pages = printer.pageList();
	 *     // print the pages
	 *     ...
	 *   }
	 * }
	 * \endcode
	 * @param p the current page number (starting from 1)
	 */
	void setCurrentPage(int p = 0);
	/**
	 * Returns the page set of the current KPrinter object.
	 */
	PageSetType pageSet() const;
	/**
	 * Sets up the KPrinter object using the print dialog, returns true if the user clicked OK.
	 * @param parent the parent widget to use for the print dialog
	 * @param caption the caption to use in the print dialog
	 * @param forceExpand force the expansion of the dialog (the show/hide button will be hidden)
	 * @returns boolean value corresponding to the button clicked by the user
	 */
	bool setup(TQWidget *parent = 0, const TQString& caption = TQString::null, bool forceExpand = false);
	/**
	 * See TQPrinter::margins().
	 */
	TQSize margins() const;
	/**
	 * Not used yet.
	 */
	void setMargins(TQSize m);
	/**
	 * Returns the page size in dot unit ( 1 dot = 1/72th in ). This method is intended for
	 * internal use, if you want to access actual page size, use a TQPaintDeviceMetrics object
	 * instead. Note that the size returned by this method does not take resolution into
	 * account, and that it can returns invalid size if no page metric was found in the printer
	 * driver. DO NOT USE, WILL BE REMOVED.
	 * @see setRealPageSize
	 * @obsolete
	 */
	TQSize realPageSize() const;
	/**
	 * DO NOT USE, WILL BE REMOVED.
	 * @obsolete
	 */
	void setRealPageSize( TQSize p );
	/**
	 * DO NOT USE, WILL BE REMOVED.
	 * @obsolete
	 */
	void setRealDrawableArea( const TQRect& r );
	/**
	 * DO NOT USE, WILL BE REMOVED.
	 * @obsolete
	 */
	TQRect realDrawableArea() const;

	void margins( uint *top, uint *left, uint *bottom, uint *right ) const;
	void setMargins( uint top, uint left, uint bottom, uint right );

	/**
	 * Returns the page list to be printed, correpsonding to the options selected by the user. This
	 * takes into account collate, page order, page set, ranges, ... This is useful when the
	 * application does page selection itself.
	 * @see setCurrentPage()
	 */
	TQValueList<int> pageList() const;
	/**
	 * Sets the KPrinter object to preview mode if @p on is true. In this mode, nothing will be
	 * printed but only a preview dialog will be popped up with the single "Close" action. Using
	 * this mode, any application can easily implement a preview action:
	 * \code
	 * void MyClass:doPreview()
	 * {
	 *   // use a "clean" KPrinter object (independent from previous print jobs),
	 *   // this is not necessary, it depends on the application
	 *   KPrinter prt( false );
	 *   prt.setPreviewOnly( true );
	 *
	 *   TQPainter painter( &prt );
	 *   doPrint( &painter );
	 * }
	 * \endcode
	 * @param on the preview-only state
	 * @see previewOnly()
	 */
	void setPreviewOnly(bool on);
	/**
	 * Returns the preview-only state for this KPrinter object.
	 * @see setPreviewOnly()
	 */
	bool previewOnly() const;
	/**
	 * Set the resolution of the current KPrinter object. The resolution is given in DPI. This
	 * resolution mainly affects the accuracy for object positionning on the paint device, and
	 * does not influence the real resolution that will be used by the printer (this should be
	 * set in the driver settings). The resolution is usually defined in the constructor.
	 * @param dpi the resolution in DPI
	 * @see KPrinter(), resolution()
	 */
	void setResolution(int dpi);
	/**
	 * Resturns the resolution of the current KPrinter object. The resolution is given in DPI.
	 * @returns resolution in DPI
	 * @see setResolution(), KPrinter()
	 */
	int resolution() const;

	/**
	 * Define the KPrinter object to use the actual printer resolution. Under some print systems
	 * (CUPS, Foomatic, PostScript printers), it is possible to know the actual resolution that
	 * is used by the printer, as selected by the user in the driver settings. If @p on is true,
	 * this KPrinter object will use the actual printer resolution if it is able to extract it.
	 * If nothing can be found, the default resolution will be the one defined by the PrinterMode
	 * argument used in the KPrinter constructor, or set explicitly by setResolution().
	 * @param on true if the KPrinter object should use the actual printer resolution
	 * @see resolution(), setResolution()
	 */
	void setUsePrinterResolution( bool on );

	/**
	 * For internal use only.
	 */
	KPrinterImpl* implementation() const;
	/**
	 * Prints the files given in argument. This will first filter the files accorsing to the filtering
	 * options selected by the user in the print dialog, then send the filtered files to the printer
	 * with the print options selected. This function is called automatically when calling
	 * TQPainter::end() for a painter object constructed on KPrinter. In normal use, you don't need
	 * this use this function explicitly.
	 */
	bool printFiles(const TQStringList& files, bool removeafter = false, bool startviewer = true);

	/**
	 * Adds a customized page to the print dialog. The pages will appear in a tab widget in the
	 * bottom half of the dialog, along with the standard "Copies" page. The page must be created
	 * and added each time you want open a print dialog with setup(). If you correctly
	 * reimplemented KPrintDialogPage::setOptions(), the settings will be restored from call
	 * to call, you don't have to worry about state saving/restoration.
	 * @param _page the page to add
	 * @see KPrintDialogPage::setOptions()
	 */
	static void addDialogPage(KPrintDialogPage* _page);
	/**
	 * Sets the page selection mode of the application. If needed, call this method somewhere at
	 * the beginning of your code. The default value is @p SystemSide.
	 * @param _mode the mode for the application
	 * @see pageSelection()
	 */
	static void setPageSelection(PageSelectionType _mode);
	/**
	 * Returns the page selection mode of the current application.
	 * @returns the page selection mode
	 * @see setPageSelection()
	 */
	static PageSelectionType pageSelection();
	/**
	 * Sets the application type concerning the print dialog. You probably don't want to use it.
	 * For internal use only.
	 * @param type the type for this application
	 * @see applicationType()
	 */
	static void setApplicationType(ApplicationType type);
	/**
	 * Returns the application type concerning the print dialog. For internal use only.
	 * @returns the type for the current application
	 * @see setApplicationType()
	 */
	static ApplicationType applicationType();
	/**
	 * Adds a standard page to the print dialog. This is not useful yet as there's only one
	 * standard page defines @p CopiesPage.
	 * @param p the page identifier
	 * @see StandardPageType
	 */
	static void addStandardPage(int p);
	/**
	 * Removes a standard page from the print dialog. If your application doesn't want a
	 * standard page in the dialog, simply call this method with the correct identifier.
	 * By default, the print dialog includes the @p CopiesPage page.
	 * @param p the page identifier
	 * @see StandardPageType
	 */
	static void removeStandardPage(int p);
	/**
	 * Starts the add printer wizard. This utility function allows any application for
	 * adding a printer using the TDEPrint powerful wizard.
	 * @param parent the parent widget for the wizard
	 * @returns 1: success, 0: cancel, -1: error
	 */
	//static int addPrinterWizard(TQWidget *parent = 0);

	/**
	 * The KPrinter object stores all its settings in an internal TQMap structure on
	 * TQString. This allows to store any property. This method allows an application to access
	 * any print option from the KPrinter object, using the option name. For example if your
	 * application add a customized page to the print dialog, this page will saves its settings
	 * into the KPrinter object using this TQMap<TQString,TQString> structure. After showing the
	 * print dialog, your application can then access these options using this method. The
	 * custom option name should follow the form "kde-appname-optionname".
	 * \code
	 * void MyClass::doPrint()
	 * {
	 *   KPrinter prt;
	 *
	 *   // add my custom page
	 *   prt.addDialogPage( new MyDialogPage() );
	 *
	 *   // open print dialog
	 *   if ( prt.setup( this ) )
	 *   {
	 *      TQString fntname = prt.option( "kde-myapp-fontname" );
	 *      ...
	 *      do_something;
	 *      ...
	 *   }
	 * }
	 * \endcode
	 * @param key the option name (key)
	 * @returns the option value correponding to the key, or TQString::null
	 * @see KPrintDialogPage, setOption, options(), setOptions()
	 */
	const TQString& option(const TQString& key) const;
	/**
	 * Adds or modifies an option in the KPrinter object. You probably don't need to use this function
	 * explicitly. This will be done implicitely for example when reimplementing
	 * KPrintDialogPage::getOptions().
	 * @param key the option name
	 * @param value the option value
	 * @see option(), KPrintDialogPage
	 */
	void setOption(const TQString& key, const TQString& value);
	/**
	 * Returns the complete set of print options from the KPrinter object. For internal use.
	 * @returns the option set as a TQMap object
	 */
	const TQMap<TQString,TQString>& options() const;
	/**
	 * Sets the option set in one operation. This method has some side effects like merging
	 * the internal map with the one given in argument, but only for option in the form
	 * "kde-...". For internal use only.
	 * @param opts the option set to be merged in the KPrinter object
	 */
	void setOptions(const TQMap<TQString,TQString>& opts);
	/**
	 * For internal use only. Does a similar job as setOption(), except that all possible
	 * internal printers are initialized with the option if it has the form "kde-...".
	 * @param opts the option set
	 * @see setOptions()
	 */
	void initOptions(const TQMap<TQString,TQString>& opts);

	/**
	 * Returns the search name of the printer selected by the user. Each printer is identified by
	 * a unique name. This method is mainly used for state restoration. For internal use.
	 * @returns the unique printer search name
	 * @see setSearchName
	 */
	TQString searchName() const;
	/**
	 * Sets the search name of the KPrinter object. For internal use.
	 * @param n the unique printer search name
	 * @see searchName()
	 */
	void setSearchName(const TQString& n);
	/**
	 * Returns the last error message issued by the print system. Unimplemented yet.
	 * @returns the last error message
	 */
	TQString errorMessage() const;
	/**
	 * Sets the last error message. For internal use.
	 * @param msg the error message
	 */
	void setErrorMessage(const TQString& msg);
	/**
	 * Configure the KPrinter object to be used with the printer named
	 * @p prname. After the call, the KPrinter object can be used without
	 * the need to call the print dialog. If @p prname is empty, then the
	 * KPrinter object is configured for the default printer. If @p prname
	 * corresponds to a pseudo-printer which needs an output file, a file
	 * dialog will be used. In that case, providing a parent widget for
	 * that dialog in @p parent may be useful.
	 * @param prname the name of the printer for which the KPrinter object
	 * has to be configured
	 * @param parent a parent widget, used a parent for a file dialog
	 * @returns boolean flag: if false, the KPrinter has not been correctly
	 * set up, and the application shouldn't use it to print. This may
	 * happen if the printer named @p prname has not been found or if the
	 * user clicked "Cancel" in the file dialog.
	 * @see setup()
	 */
	bool autoConfigure(const TQString& prname = TQString::null, TQWidget *parent = 0);
	/**
	 * Set the default document filename. This filename will be used as the
	 * default basename for the output file, instead of the default "print".
	 * For example, by using setDocFileName("my_document"), the default
	 * output file will be $HOME/my_document.ps.
	 * @param filename the default document basename to use
	 * @see docFileName()
	 */
	void setDocFileName(const TQString& filename);
	/**
	 * Get the default document filename, that is the default basename used for
	 * the output file.
	 * @returns the default document basename
	 * @see setDocFileName()
	 */
	TQString docFileName() const;
	/**
	 * Set the default document directory. This directory will be used as
	 * the default location for any output file. If not set, $HOME directory
	 * is used instead.
	 * @param dir the new default output directory
	 * @see docDirectory()
	 */
	void setDocDirectory( const TQString& dir );
	/**
	 * Get the default document directory, that is the directory used for
	 * any output file. By default, it is the $HOME directory.
	 * @returns the default output directory
	 * @see setDocDirectory
	 */
	TQString docDirectory() const;

protected:
	virtual bool cmd(int, TQPainter*, TQPDevCmdParam*);
	virtual int metric(int) const;
	void translateQtOptions();
	void loadSettings();
	void saveSettings();
	void preparePrinting();
	void finishPrinting();
	void reload();
	void init(bool restore = true, TQPrinter::PrinterMode m = TQPrinter::ScreenResolution);
	bool doPreview(const TQString& file);
	void setRealPageSize(DrPageSize* p);
	void setOption( const TQString& key, const TQString& value, bool broadcast );

protected:
	KPrinterPrivate		*d;
};

//**************************************************************************************

// Utility functions

TDEPRINT_EXPORT KPrinter::PageSize pageNameToPageSize(const TQString& name);
TDEPRINT_EXPORT const char* pageSizeToPageName(KPrinter::PageSize s);
/**
 * DO NOT USE, WILL BE REMOVED.
 * @obsolete
 */
TQSize rangeToSize( const TQString& );

#endif
