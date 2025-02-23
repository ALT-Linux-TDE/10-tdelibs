#ifndef CONFIG_TDEFILE_H
#define CONFIG_TDEFILE_H

const int tdefile_area = 250;

#define DefaultViewStyle TQString::fromLatin1("SimpleView")
#define DefaultPannerPosition 40
#define DefaultMixDirsAndFiles false
#define DefaultShowStatusLine false
#define DefaultShowHidden false
#define DefaultCaseInsensitive true
#define DefaultDirsFirst true
#define DefaultSortReversed false
#define DefaultRecentURLsNumber 15
#define DefaultDirectoryFollowing true
#define DefaultAutoSelectExtChecked true
#define ConfigGroup TQString::fromLatin1("KFileDialog Settings")
#define RecentURLs TQString::fromLatin1("Recent URLs")
#define RecentFiles TQString::fromLatin1("Recent Files")
#define RecentURLsNumber TQString::fromLatin1("Maximum of recent URLs")
#define RecentFilesNumber TQString::fromLatin1("Maximum of recent files")
#define DialogWidth TQString::fromLatin1("Width (%1)")
#define DialogHeight TQString::fromLatin1("Height (%1)")
#define ConfigShowStatusLine TQString::fromLatin1("ShowStatusLine")
#define AutoDirectoryFollowing TQString::fromLatin1("Automatic directory following")
#define PathComboCompletionMode TQString::fromLatin1("PathCombo Completionmode")
#define LocationComboCompletionMode TQString::fromLatin1("LocationCombo Completionmode")
#define ShowSpeedbar TQString::fromLatin1("Show Speedbar")
#define ShowBookmarks TQString::fromLatin1("Show Bookmarks")
#define AutoSelectExtChecked TQString::fromLatin1("Automatically select filename extension")

#endif
