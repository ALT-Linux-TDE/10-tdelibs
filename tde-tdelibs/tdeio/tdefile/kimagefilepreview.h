/*
 *
 * This file is part of the KDE project.
 * Copyright (C) 2001 Martin R. Jones <mjones@kde.org>
 *               2001 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 * You can Freely distribute this program under the GNU Library General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef KIMAGEFILEPREVIEW_H
#define KIMAGEFILEPREVIEW_H

#include <tqpixmap.h>

#include <kurl.h>
#include <kpreviewwidgetbase.h>

class TQCheckBox;
class TQPushButton;
class TQLabel;
class TQTimer;

class KFileDialog;
class KFileItem;
namespace TDEIO { class Job; class PreviewJob; }

/**
 * Image preview widget for the file dialog.
 */
class TDEIO_EXPORT KImageFilePreview : public KPreviewWidgetBase
{
	TQ_OBJECT

	public:
		KImageFilePreview(TQWidget *parent);
		~KImageFilePreview();

		virtual TQSize sizeHint() const;

	public slots:
		virtual void showPreview(const KURL &url);
		virtual void clearPreview();

	protected slots:
		void showPreview();
		void showPreview( const KURL& url, bool force );

		void toggleAuto(bool);
		virtual void gotPreview( const KFileItem*, const TQPixmap& );

	protected:
		virtual void resizeEvent(TQResizeEvent *e);
		virtual TDEIO::PreviewJob * createJob( const KURL& url,
                                                     int w, int h );

	private slots:
		void slotResult( TDEIO::Job * );
		virtual void slotFailed( const KFileItem* );

	private:
		bool autoMode;
		KURL currentURL;
		TQTimer *timer;
		TQLabel *imageLabel;
		TQLabel *infoLabel;
		TQCheckBox *autoPreview;
		TQPushButton *previewButton;
		TDEIO::PreviewJob *m_job;
        protected:
                virtual void virtual_hook( int id, void* data );
        private:
                class KImageFilePreviewPrivate;
                KImageFilePreviewPrivate *d;
};

#endif // KIMAGEFILEPREVIEW_H
