/*
 * Copyright (c) 2002,2003 Hamish Rodda <rodda@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __RANDR_H__
#define __RANDR_H__

#include <tqobject.h>
#include <tqstringlist.h>
#include <tqptrlist.h>

#include <tdecmodule.h>
#include <tdeconfig.h>

#define ROTATION_0_DEGREES_INDEX 0
#define ROTATION_90_DEGREES_INDEX 1
#define ROTATION_180_DEGREES_INDEX 2
#define ROTATION_270_DEGREES_INDEX 3

class KTimerDialog;
class RandRScreenPrivate;

class TDERANDR_EXPORT HotPlugRule {
	public:
		enum states {
			AnyState		= 0,
			Connected		= 1,
			Disconnected		= 2
		};

	public:
		HotPlugRule();
		virtual ~HotPlugRule();

	public:
		TQStringList outputs;
		TQValueList< int > states;
		TQString profileName;
};

typedef TQValueList< HotPlugRule > HotPlugRulesList;

class TDERANDR_EXPORT SingleScreenData {
	public:
		SingleScreenData();
		virtual ~SingleScreenData();
	
	public:
		TQString screenUniqueName;
		TQString screenFriendlyName;
		bool generic_screen_detected;
		bool screen_connected;
	
		TQStringList resolutions;
		TQStringList refresh_rates;
		TQStringList color_depths;
		TQStringList rotations;
	
		int current_resolution_index;
		int current_refresh_rate_index;
		int current_color_depth_index;
	
		float gamma_red;
		float gamma_green;
		float gamma_blue;
	
		int current_rotation_index;
		int current_orientation_mask;
		bool has_x_flip;
		bool has_y_flip;
		bool supports_transformations;
	
		bool is_primary;
		bool is_extended;
		int absolute_x_position;
		int absolute_y_position;
		int current_x_pixel_count;
		int current_y_pixel_count;
	
		bool has_dpms;
		bool enable_dpms;
		unsigned int dpms_standby_delay;
		unsigned int dpms_suspend_delay;
		unsigned int dpms_off_delay;
};

class RandRScreen : public TQObject
{
	TQ_OBJECT

public:
	enum orientations {
		Rotate0			= 0x1,
		Rotate90		= 0x2,
		Rotate180		= 0x4,
		Rotate270		= 0x8,
		RotateMask		= 15,
		RotationCount	= 4,
		ReflectX		= 0x10,
		ReflectY		= 0x20,
		ReflectMask		= 48,
		OrientationMask	= 63,
		OrientationCount = 6
	};

	RandRScreen(int screenIndex);
	~RandRScreen();

	void		loadSettings();
	void		setOriginal();

	bool		applyProposed();

	/**
	 * @returns false if the user did not confirm in time, or cancelled, or the change failed
	 */
	bool		applyProposedAndConfirm();

public slots:
	bool		confirm();
	bool		showTestConfigurationDialog();

public:
	TQString		changedMessage() const;

	bool		changedFromOriginal() const;
	void		proposeOriginal();

	bool		proposedChanged() const;

	static TQString	rotationName(int rotation, bool pastTense = false, bool capitalised = true);
	TQPixmap	        rotationIcon(int rotation) const;
	TQString			currentRotationDescription() const;

	int				rotationIndexToDegree(int rotation) const;
	int				rotationDegreeToIndex(int degree) const;

	/**
	 * Refresh rate functions.
	 */
	TQStringList refreshRates(int size) const;

	TQString		refreshRateDirectDescription(int rate) const;
	TQString		refreshRateIndirectDescription(int size, int index) const;
	TQString		refreshRateDescription(int size, int index) const;

	int			currentRefreshRate() const;
	TQString		currentRefreshRateDescription() const;

	// Refresh rate hz <==> index conversion
	int			refreshRateHzToIndex(int size, int hz) const;
	int			refreshRateIndexToHz(int size, int index) const;

	/**
	 * Screen size functions.
	 */
	int				numSizes() const;
	const TQSize&	pixelSize(int index) const;
	const TQSize&	mmSize(int index) const;
	int				pixelCount(int index) const;

	/**
	 * Retrieve the index of a screen size with a specified pixel size.
	 *
	 * @param pixelSize dimensions of the screen in pixels
	 * @returns the index of the requested screen size
	 */
	int				sizeIndex(TQSize pixelSize) const;

	int			rotations() const;

	/**
	 * Current setting functions.
	 */
	int			currentPixelWidth() const;
	int			currentPixelHeight() const;
	int			currentMMWidth() const;
	int			currentMMHeight() const;

	int			currentRotation() const;
	int			currentSize() const;

	/**
	 * Proposed setting functions.
	 */
	int			proposedSize() const;
	bool		proposeSize(int newSize);

	int			proposedRotation() const;
	void		proposeRotation(int newRotation);

	int			proposedRefreshRate() const;
	/**
	 * Propose a refresh rate.
	 * Please note that you must propose the target size first for this to work.
	 *
	 * @param index the index of the refresh rate (not a refresh rate in hz!)
	 * @returns true if successful, false otherwise.
	 */
	bool		proposeRefreshRate(int index);

	/**
	 * Configuration functions.
	 */
	void		load(TDEConfig& config);
	void		save(TDEConfig& config) const;

private:
	RandRScreenPrivate*	d;

	int			m_screen;

	TQValueList<TQSize>	m_pixelSizes;
	TQValueList<TQSize>	m_mmSizes;
	int					m_rotations;

	int			m_originalRotation;
	int			m_originalSize;
	int			m_originalRefreshRate;

	int			m_currentRotation;
	int			m_currentSize;
	int			m_currentRefreshRate;

	int			m_proposedRotation;
	int			m_proposedSize;
	int			m_proposedRefreshRate;

	KTimerDialog*	m_shownDialog;

private slots:
	void		desktopResized();
	void		shownDialogDestroyed();
};

typedef TQPtrList<RandRScreen> ScreenList;

class RandRDisplay
{
public:
	RandRDisplay();

	bool			isValid() const;
	const TQString&	errorCode() const;
	const TQString&	version() const;

	int		eventBase() const;
	int		screenChangeNotifyEvent() const;
	int		errorBase() const;

	int		screenIndexOfWidget(TQWidget* widget);

	int				numScreens() const;
	RandRScreen*	screen(int index);

	void			setCurrentScreen(int index);
	int				currentScreenIndex() const;
	RandRScreen*	currentScreen();

	void	refresh();

	/**
	 * Loads saved settings.
	 *
	 * @param config the TDEConfig object to load from
	 * @param loadScreens whether to call RandRScreen::load() for each screen
	 * @retuns true if the settings should be applied on KDE startup.
	 */
	bool	loadDisplay(TDEConfig& config, bool loadScreens = true);
	void	saveDisplay(TDEConfig& config, bool applyOnStartup, bool syncTrayApp);

	static bool		applyOnStartup(TDEConfig& config);
	static bool		syncTrayApp(TDEConfig& config);

	void	applyProposed(bool confirm = true);

	bool showTestConfigurationDialog();

private:
	int				m_numScreens;
	int				m_currentScreenIndex;
	RandRScreen*	m_currentScreen;
	ScreenList		m_screens;

	bool			m_valid;
	TQString			m_errorCode;
	TQString			m_version;

	int				m_eventBase;
	int				m_errorBase;
};

#endif
