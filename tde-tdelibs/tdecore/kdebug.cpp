/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
                  2002 Holger Freyther (freyther@kde.org)

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

#include "kdebug.h"

#ifdef NDEBUG
#undef kdDebug
#endif

#include "kdebugdcopiface.h"

#include "tdeapplication.h"
#include "tdeglobal.h"
#include "kinstance.h"
#include "kstandarddirs.h"

#include <tqmessagebox.h>
#include <tdelocale.h>
#include <tqfile.h>
#include <tqintdict.h>
#include <tqstring.h>
#include <tqdatetime.h>
#include <tqpoint.h>
#include <tqrect.h>
#include <tqregion.h>
#include <tqstringlist.h>
#include <tqpen.h>
#include <tqbrush.h>
#include <tqsize.h>

#include <kurl.h>

#include <stdlib.h>	// abort
#include <unistd.h>	// getpid
#include <stdarg.h>	// vararg stuff
#include <ctype.h>      // isprint
#include <syslog.h>
#include <errno.h>
#include <cstring>
#include <tdeconfig.h>
#include "kstaticdeleter.h"
#include <config.h>

#ifdef HAVE_BACKTRACE
#include BACKTRACE_H

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#ifdef HAVE_ABI_CXA_DEMANGLE
#include <cxxabi.h>
#endif

#include <link.h>
#ifdef WITH_LIBBFD
/* newer versions of libbfd require some autotools-specific macros to be defined */
/* see binutils Bug 14243 and 14072 */
#define PACKAGE tdelibs
#define PACKAGE_VERSION TDE_VERSION

#include <bfd.h>

#ifdef HAVE_DEMANGLE_H
#include <demangle.h>
#endif // HAVE_DEMANGLE_H
#endif // WITH_LIBBFD

#endif // HAVE_BACKTRACE

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif // HAVE_ALLOCA_H

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif // HAVE_STDINT_H

class KDebugEntry;

class KDebugEntry
{
public:
    KDebugEntry (int n, const TQCString& d) {number=n; descr=d;}
    unsigned int number;
    TQCString descr;
};

static TQIntDict<KDebugEntry> *KDebugCache;

static KStaticDeleter< TQIntDict<KDebugEntry> > kdd;

static TQCString getDescrFromNum(unsigned int _num)
{
  if (!KDebugCache) {
    kdd.setObject(KDebugCache, new TQIntDict<KDebugEntry>( 601 ));
    // Do not call this deleter from ~TDEApplication
    TDEGlobal::unregisterStaticDeleter(&kdd);
    KDebugCache->setAutoDelete(true);
  }

  KDebugEntry *ent = KDebugCache->find( _num );
  if ( ent )
    return ent->descr;

  if ( !KDebugCache->isEmpty() ) // areas already loaded
    return TQCString();

  TQString filename(locate("config","kdebug.areas"));
  if (filename.isEmpty())
      return TQCString();

  TQFile file(filename);
  if (!file.open(IO_ReadOnly)) {
    tqWarning("Couldn't open %s", filename.local8Bit().data());
    file.close();
    return TQCString();
  }

  uint lineNumber=0;
  TQCString line(1024);
  int len;

  while (( len = file.readLine(line.data(),line.size()-1) ) > 0) {
      int i=0;
      ++lineNumber;

      while (line[i] && line[i] <= ' ')
        i++;

      unsigned char ch=line[i];

      if ( !ch || ch =='#' || ch =='\n')
          continue; // We have an eof, a comment or an empty line

      if (ch < '0' && ch > '9') {
          tqWarning("Syntax error: no number (line %u)",lineNumber);
          continue;
      }

      const int numStart=i;
      do {
          ch=line[++i];
      } while ( ch >= '0' && ch <= '9');

      const TQ_ULONG number =line.mid(numStart,i).toULong();

      while (line[i] && line[i] <= ' ')
        i++;

      KDebugCache->insert(number, new KDebugEntry(number, line.mid(i, len-i-1)));
  }
  file.close();

  ent = KDebugCache->find( _num );
  if ( ent )
      return ent->descr;

  return TQCString();
}

enum DebugLevels {
    KDEBUG_INFO=    0,
    KDEBUG_WARN=    1,
    KDEBUG_ERROR=   2,
    KDEBUG_FATAL=   3
};


struct kDebugPrivate {
  kDebugPrivate() :
  	oldarea(0), config(0) { }

  ~kDebugPrivate() { delete config; }

  TQCString aAreaName;
  unsigned int oldarea;
  TDEConfig *config;
};

static kDebugPrivate *kDebug_data = 0;
static KStaticDeleter<kDebugPrivate> pcd;
static KStaticDeleter<KDebugDCOPIface> dcopsd;
static KDebugDCOPIface* kDebugDCOPIface = 0;

static void kDebugBackend( unsigned short nLevel, unsigned int nArea, const char *data)
{
  if ( !kDebug_data )
  {
      pcd.setObject(kDebug_data, new kDebugPrivate());
      // Do not call this deleter from ~TDEApplication
      TDEGlobal::unregisterStaticDeleter(&pcd);

      // create the dcop interface if it has not been created yet
      if (!kDebugDCOPIface)
      {
          kDebugDCOPIface = dcopsd.setObject(kDebugDCOPIface, new KDebugDCOPIface);
      }
  }

  if (!kDebug_data->config && TDEGlobal::_instance )
  {
      kDebug_data->config = new TDEConfig("kdebugrc", false, false);
      kDebug_data->config->setGroup("0");

      //AB: this is necessary here, otherwise all output with area 0 won't be
      //prefixed with anything, unless something with area != 0 is called before
      if ( TDEGlobal::_instance )
        kDebug_data->aAreaName = TDEGlobal::instance()->instanceName();
  }

  if ( kDebug_data->oldarea != nArea ) {
    kDebug_data->oldarea = nArea;
    if( TDEGlobal::_instance ) {
      if ( nArea > 0 ) {
        kDebug_data->aAreaName = getDescrFromNum(nArea);
      }
      if ( nArea == 0 || kDebug_data->aAreaName.isEmpty() ) {
        kDebug_data->aAreaName = TDEGlobal::instance()->instanceName();
      }
    }
  }

  int nPriority = 0;
  TQString aCaption;

    /* Determine output */

  TQString key;
  switch( nLevel )
  {
  case KDEBUG_INFO:
      key = "InfoOutput";
      aCaption = "Info";
      nPriority = LOG_INFO;
      break;
  case KDEBUG_WARN:
      key = "WarnOutput";
      aCaption = "Warning";
      nPriority = LOG_WARNING;
    break;
  case KDEBUG_FATAL:
      key = "FatalOutput";
      aCaption = "Fatal Error";
      nPriority = LOG_CRIT;
      break;
  case KDEBUG_ERROR:
  default:
      /* Programmer error, use "Error" as default */
      key = "ErrorOutput";
      aCaption = "Error";
      nPriority = LOG_ERR;
      break;
  }

  short nOutput = -1;
  if ( kDebug_data->config ) {
    kDebug_data->config->setGroup( TQString::number(static_cast<int>(nArea)) );
    nOutput = kDebug_data->config->readNumEntry(key, -1);
    if( nOutput == -1 ) {
      kDebug_data->config->setGroup( TQString::fromAscii("Default") );
      nOutput = kDebug_data->config->readNumEntry(key, -1);
    }
  }
  // if no output mode is specified default to no stderr output
  // NOTE: don't set this to 4 (no output) because in that case you won't be
  //       able to get any output from applications which don't create
  //       TDEApplication objects.
  if ( nOutput == -1 ) {
    nOutput = 2;
  }

  // If the application doesn't have a TQApplication object it can't use
  // a messagebox, as well as in case of GUI is disabled.
  if ( nOutput == 1 && ( !kapp || !kapp->guiEnabled()) ) {
    nOutput = 2;
  } else if ( nOutput == 4 && nLevel != KDEBUG_FATAL ) {
      return;
  }

  const int BUF_SIZE = 4096;
  const int BUF_PID_SIZE = 20;
  char buf[BUF_SIZE];
  char buf_pid[BUF_PID_SIZE];
  strlcpy(buf, TQDateTime::currentDateTime().toString("[yyyy/MM/dd hh:mm:ss.zzz] ").ascii(), BUF_SIZE);
  if (!kDebug_data->aAreaName.isEmpty())
  {
    strlcat( buf, "[", BUF_SIZE );
    strlcat( buf, kDebug_data->aAreaName.data(), BUF_SIZE );
    strlcat( buf, "] ", BUF_SIZE );
  }
  snprintf(buf_pid, BUF_PID_SIZE, "[%d] ", getpid());
  strlcat(buf, buf_pid, BUF_SIZE);
  strlcat(buf, data, BUF_SIZE);

  // Output
  switch( nOutput )
  {
  case 0: // File
  {
      const char* aKey;
      switch( nLevel )
      {
      case KDEBUG_INFO:
          aKey = "InfoFilename";
          break;
      case KDEBUG_WARN:
          aKey = "WarnFilename";
          break;
      case KDEBUG_FATAL:
          aKey = "FatalFilename";
          break;
      case KDEBUG_ERROR:
      default:
          aKey = "ErrorFilename";
          break;
      }
      TQFile aOutputFile( kDebug_data->config->readPathEntry(aKey, "kdebug.dbg") );
      aOutputFile.open( IO_WriteOnly | IO_Append | IO_Raw );
      aOutputFile.writeBlock( buf, strlen( buf ) );
      aOutputFile.close();
      break;
  }
  case 1: // Message Box
  {
      // Since we are in tdecore here, we cannot use KMsgBox and use
      // TQMessageBox instead
      if ( !kDebug_data->aAreaName.isEmpty() )
          aCaption += TQString("(%1)").arg( TQString(kDebug_data->aAreaName) );
      TQMessageBox::warning( 0L, aCaption, data, i18n("&OK") );
      break;
  }
  case 2: // Shell
  {
      if (write( 2, buf, strlen( buf ) ) < 0) {  //fputs( buf, stderr );
          // ERROR
      }
      break;
  }
  case 3: // syslog
  {
      syslog( nPriority, "%s", buf);
      break;
  }
  }

  // check if we should abort
  if( ( nLevel == KDEBUG_FATAL )
      && ( !kDebug_data->config || kDebug_data->config->readNumEntry( "AbortFatal", 1 ) ) )
        abort();
}

kdbgstream& perror( kdbgstream &s) { return s << TQString(TQString::fromLocal8Bit(strerror(errno))); }
kdbgstream kdDebug(int area) { return kdbgstream(area, KDEBUG_INFO); }
kdbgstream kdDebug(bool cond, int area) { if (cond) return kdbgstream(area, KDEBUG_INFO); else return kdbgstream(0, 0, false); }

kdbgstream kdError(int area) { return kdbgstream("ERROR: ", area, KDEBUG_ERROR); }
kdbgstream kdError(bool cond, int area) { if (cond) return kdbgstream("ERROR: ", area, KDEBUG_ERROR); else return kdbgstream(0,0,false); }
kdbgstream kdWarning(int area) { return kdbgstream("WARNING: ", area, KDEBUG_WARN); }
kdbgstream kdWarning(bool cond, int area) { if (cond) return kdbgstream("WARNING: ", area, KDEBUG_WARN); else return kdbgstream(0,0,false); }
kdbgstream kdFatal(int area) { return kdbgstream("FATAL: ", area, KDEBUG_FATAL); }
kdbgstream kdFatal(bool cond, int area) { if (cond) return kdbgstream("FATAL: ", area, KDEBUG_FATAL); else return kdbgstream(0,0,false); }

kdbgstream::kdbgstream(kdbgstream &str)
 : output(str.output), area(str.area), level(str.level), print(str.print) 
{ 
    str.output.truncate(0); 
}

void kdbgstream::flush() {
    if (output.isEmpty() || !print)
	return;
    kDebugBackend( level, area, output.local8Bit().data() );
    output = TQString::null;
}

kdbgstream &kdbgstream::form(const char *format, ...)
{
    char buf[4096];
    va_list arguments;
    va_start( arguments, format );
    vsnprintf( buf, sizeof(buf), format, arguments );
    va_end(arguments);
    *this << buf;
    return *this;
}

kdbgstream::~kdbgstream() {
    if (!output.isEmpty()) {
	fprintf(stderr, "ASSERT: debug output not ended with \\n\n");
        TQString backtrace = kdBacktrace();
        if (backtrace.ascii() != NULL) {
                fprintf(stderr, "%s", backtrace.latin1());
        }
	*this << '\n';
    }
}

kdbgstream& kdbgstream::operator<< (char ch)
{
  if (!print) return *this;
  if (!isprint(ch))
    output += "\\x" + TQString::number( static_cast<uint>( ch ), 16 ).rightJustify(2, '0');
  else {
    output += ch;
    if (ch == '\n') flush();
  }
  return *this;
}

kdbgstream& kdbgstream::operator<< (TQChar ch)
{
  if (!print) return *this;
  if (!ch.isPrint())
    output += "\\x" + TQString::number( ch.unicode(), 16 ).rightJustify(2, '0');
  else {
    output += ch;
    if (ch == TQChar('\n')) flush();
  }
  return *this;
}

kdbgstream& kdbgstream::operator<< (TQWidget* widget)
{
    return *this << const_cast< const TQWidget* >( widget );
}

kdbgstream& kdbgstream::operator<< (const TQWidget* widget)
{
  TQString string, temp;
  // -----
  if(widget==0)
    {
      string=(TQString)"[Null pointer]";
    } else {
      temp.setNum((ulong)widget, 16);
      string=(TQString)"["+widget->className()+" pointer "
	+ "(0x" + temp + ")";
      if(widget->name(0)==0)
	{
	  string += " to unnamed widget, ";
	} else {
	  string += (TQString)" to widget " + widget->name() + ", ";
	}
      string += "geometry="
	+ TQString().setNum(widget->width())
	+ "x"+TQString().setNum(widget->height())
	+ "+"+TQString().setNum(widget->x())
	+ "+"+TQString().setNum(widget->y())
	+ "]";
    }
  if (!print)
    {
      return *this;
    }
  output += string;
  if (output.at(output.length() -1 ) == TQChar('\n'))
    {
      flush();
    }
  return *this;
}
/*
 * either use 'output' directly and do the flush if needed
 * or use the TQString operator which calls the char* operator
 *
 */
kdbgstream& kdbgstream::operator<<( const TQDateTime& time) {
    *this << time.toString();
    return *this;
}
kdbgstream& kdbgstream::operator<<( const TQDate& date) {
    *this << TQString(date.toString());

    return *this;
}
kdbgstream& kdbgstream::operator<<( const TQTime& time ) {
    *this << TQString(time.toString());
    return *this;
}
kdbgstream& kdbgstream::operator<<( const TQPoint& p ) {
    *this << "(" << p.x() << ", " << p.y() << ")";
    return *this;
}
kdbgstream& kdbgstream::operator<<( const TQSize& s ) {
    *this << "[" << s.width() << "x" << s.height() << "]";
    return *this;
}
kdbgstream& kdbgstream::operator<<( const TQRect& r ) {
    *this << "[" << r.x() << "," << r.y() << " - " << r.width() << "x" << r.height() << "]";
    return *this;
}
kdbgstream& kdbgstream::operator<<( const TQRegion& reg ) {
    *this<< "[ ";

    TQMemArray<TQRect>rs=reg.rects();
    for (uint i=0;i<rs.size();++i)
        *this << TQString(TQString("[%1,%2 - %3x%4] ").arg(rs[i].x()).arg(rs[i].y()).arg(rs[i].width()).arg(rs[i].height() )) ;

    *this <<"]";
    return *this;
}
kdbgstream& kdbgstream::operator<<( const KURL& u ) {
    *this << u.prettyURL();
    return *this;
}
kdbgstream& kdbgstream::operator<<( const TQStringList& l ) {
    *this << "(";
    *this << l.join(",");
    *this << ")";

    return *this;
}
kdbgstream& kdbgstream::operator<<( const TQColor& c ) {
    if ( c.isValid() )
        *this << TQString(c.name());
    else
        *this << "(invalid/default)";
    return *this;
}
kdbgstream& kdbgstream::operator<<( const TQPen& p ) {
    static const char* const s_penStyles[] = {
        "NoPen", "SolidLine", "DashLine", "DotLine", "DashDotLine",
        "DashDotDotLine" };
    static const char* const s_capStyles[] = {
        "FlatCap", "SquareCap", "RoundCap" };
    *this << "[ style:";
    *this << s_penStyles[ p.style() ];
    *this << " width:";
    *this << p.width();
    *this << " color:";
    if ( p.color().isValid() )
        *this << TQString(p.color().name());
    else
        *this <<"(invalid/default)";
    if ( p.width() > 0 ) // cap style doesn't matter, otherwise
    {
        *this << " capstyle:";
        *this << s_capStyles[ p.capStyle() >> 4 ];
        // join style omitted
    }
    *this <<" ]";
    return *this;
}
kdbgstream& kdbgstream::operator<<( const TQBrush& b) {
    static const char* const s_brushStyles[] = {
        "NoBrush", "SolidPattern", "Dense1Pattern", "Dense2Pattern", "Dense3Pattern",
        "Dense4Pattern", "Dense5Pattern", "Dense6Pattern", "Dense7Pattern",
        "HorPattern", "VerPattern", "CrossPattern", "BDiagPattern", "FDiagPattern",
        "DiagCrossPattern" };

    *this <<"[ style: ";
    *this <<s_brushStyles[ b.style() ];
    *this <<" color: ";
    // can't use operator<<(str, b.color()) because that terminates a kdbgstream (flushes)
    if ( b.color().isValid() )
        *this << TQString(b.color().name()) ;
    else
        *this <<"(invalid/default)";
    if ( b.pixmap() )
        *this <<" has a pixmap";
    *this <<" ]";
    return *this;
}

kdbgstream& kdbgstream::operator<<( const TQVariant& v) {
    *this << "[variant: ";
    *this << v.typeName();
    // For now we just attempt a conversion to string.
    // Feel free to switch(v.type()) and improve the output.
    *this << " toString=";
    *this << v.toString();
    *this << "]";
    return *this;
}

kdbgstream& kdbgstream::operator<<( const TQByteArray& data) {
    if (!print) return *this;
    output += '[';
    unsigned int i = 0;
    unsigned int sz = TQMIN( data.size(), 64 );
    for ( ; i < sz ; ++i ) {
        output += TQString::number( (unsigned char) data[i], 16 ).rightJustify(2, '0');
        if ( i < sz )
            output += ' ';
    }
    if ( sz < data.size() )
        output += "...";
    output += ']';
    return *this;
}

#ifdef HAVE_BACKTRACE
struct BacktraceFunctionInfo {
	const void *addr;      //< the address of function returned by backtrace()
	const char* fileName;  //< the file of binary owning the function (e.g. shared library or current header)
	const void *base;      //< the base address there the binary is loaded to
	uintptr_t offset;      //< offset of the function in binary (base - address)
	TQString functionName; //< mangled name of function
	TQString prettyName;   //< demangled name of function
	TQString sourceName;   //< name of source file function declared in
	unsigned sourceLine;   //< line where function defined
};

#ifdef WITH_LIBBFD

// load symbol table from file
asymbol** bfdLoadSymtab (bfd *abfd) {
	long symCount;  // count of entries in symbol table
	long symtab_sz;      // size of the table
	asymbol** rv;
	bfd_boolean dynamic = FALSE;

	// make shure the file has symbol table
	if ((bfd_get_file_flags (abfd) & HAS_SYMS) == 0){
		return 0;
	}
	
	// determin the amount of space we'll need to store the table
	symtab_sz = bfd_get_symtab_upper_bound (abfd);
	if (symtab_sz == 0) {
		symtab_sz = bfd_get_dynamic_symtab_upper_bound (abfd);
		dynamic = TRUE;
	}
	if (symtab_sz < 0) {
		return 0;
	}
	
	// allocate memory
	rv = (asymbol **) malloc(symtab_sz); // dunno, why not malloc
	if ( !rv ) {
		return 0;
	}
	
	// actually load the table
	if (dynamic) {
		symCount = bfd_canonicalize_dynamic_symtab (abfd, rv);
	} else {
		symCount = bfd_canonicalize_symtab (abfd, rv);
	}
	
	if (symCount < 0) {
		if (rv) {
			free(rv);
		}
		return 0;
	}

	return rv;
}

void bfdFillAdditionalFunctionsInfo(BacktraceFunctionInfo &func) {
	static bool inited=0;
	if (!inited) {
		bfd_init();
		inited=1;
	}
	
	bfd *abfd = bfd_openr(func.fileName, 0); // a bfd object
	if( !abfd ) {
		return;
	}
	
	//  check format of the object
	if( !bfd_check_format(abfd, bfd_object) ) {
		bfd_close(abfd);
		return;
	} 
	
	// load symbol table
	asymbol **syms= bfdLoadSymtab(abfd);    
	if(!syms) {
		bfd_close(abfd);
		return;
	}

	// found source file and line for given address
	for (asection *sect = abfd->sections; sect != NULL; sect = sect->next) {

		if (bfd_get_section_flags(abfd, sect) & SEC_ALLOC) {
			bfd_vma sectStart = bfd_get_section_vma(abfd, sect);
			bfd_vma sectEnd   = sectStart + bfd_section_size(abfd, sect);
			if (sectStart <= func.offset && func.offset < sectEnd) {
				bfd_vma sectOffset = func.offset - sectStart;
				const char* functionName;
				const char* sourceName;
				unsigned sourceLine;
				if (bfd_find_nearest_line(abfd, sect, syms, sectOffset, 
							&sourceName, &functionName, &sourceLine))
				{
					func.sourceName   = sourceName;
					func.sourceLine   = sourceLine;
					if(func.functionName.isEmpty()) {
						func.functionName = TQString::fromAscii(functionName);
					}
					break;
				}
			}
		}
	}
#ifdef HAVE_DEMANGLE_H	
	if(func.prettyName.isEmpty() && !func.functionName.isEmpty()) {
		char *demangled = bfd_demangle(abfd, func.functionName.ascii(), DMGL_AUTO | DMGL_PARAMS);
		if (demangled) {
			func.prettyName = demangled;
			free(demangled);
		}
	}
#endif // HAVE_DEMANGLE_H	

	if( syms ) {
		free(syms);
	}
	bfd_close(abfd);
}

#endif // WITH_LIBBFD

void fillAdditionalFunctionsInfo(BacktraceFunctionInfo &func) {
#ifdef WITH_LIBBFD
	bfdFillAdditionalFunctionsInfo(func);
#endif // WITH_LIBBFD

#ifdef HAVE_ABI_CXA_DEMANGLE
	if(func.prettyName.isEmpty() && !func.functionName.isEmpty()) {
		int status=0;
		char *demangled = abi::__cxa_demangle(func.functionName.ascii(), 0, 0, &status);
		if (demangled) {
			func.prettyName = demangled;
			free(demangled);
		}
	}
#endif // HAVE_ABI_CXA_DEMANGLE

}

TQString formatBacktrace(void *addr) {
	TQString rv;
	BacktraceFunctionInfo func;
	func.addr = addr;
	
	// NOTE: if somebody would compile for some non-linux-glibc platform
	//       check if dladdr function is avalible there
	Dl_info info;

	// obtain information about the function.
#ifdef Q_OS_SOLARIS
	dladdr((void *)func.addr, &info);
#else
	dladdr(func.addr, &info);
#endif /* Solaris */

	func.fileName = info.dli_fname;
	func.base = info.dli_fbase;
	func.offset = (uintptr_t)func.addr - (uintptr_t)func.base;
	func.functionName = TQString::fromAscii(info.dli_sname);
	func.sourceLine = 0;

	fillAdditionalFunctionsInfo(func);

	rv.sprintf("0x%0*lx", (int) sizeof(void*)*2, (uintptr_t) func.addr);
	
	rv += " in ";
	if (!func.prettyName.isEmpty()) {
		rv += func.prettyName;
	} else if (!func.functionName.isEmpty()) {
		rv += func.functionName;
	} else {
		rv += "??";
	}
	
	if (!func.sourceName.isEmpty()) {
		rv += " in "; 
		rv += func.sourceName;
		rv += ":";
		rv += func.sourceLine ? TQString::number(func.sourceLine) : "??";
	} else if (func.fileName && func.fileName[0]) {
		rv += TQString().sprintf(" from %s:0x%08lx",func.fileName, func.offset);
	} else {
		rv += " from ??";
	}
	
	return rv;
}
#endif // HAVE_BACKTRACE


TQString kdBacktrace(int levels)
{
	TQString rv;
#ifdef HAVE_BACKTRACE
	if (levels < 0 || levels > 256 ) {
		levels = 256;
	}
	
	rv = "[\n";

	if (levels) {
#ifdef HAVE_ALLOCA
		void** trace = (void**)alloca(levels * sizeof(void*));
#else  // HAVE_ALLOCA
		void* trace[256];
#endif // HAVE_ALLOCA
		levels = backtrace(trace, levels);

		if (levels) { 
			for (int i = 0; i < levels; ++i) {
				rv += TQString().sprintf("#%-2d ", i);
				rv += formatBacktrace(trace[i]);
				rv += '\n';
			}
		} else {
			rv += "backtrace() failed\n";
		}
	}
	
	rv += "]\n";
#endif // HAVE_BACKTRACE
	return rv;
}

// Keep for ABI compatability for some time
// FIXME remove this (2013-08-18, 18:09, Fat-Zer)
TQString kdBacktrace()
{
    return kdBacktrace(-1 /*all*/);
}

void kdBacktraceFD(int fd) {
#ifdef HAVE_BACKTRACE
	void *trace[256];
	int levels;
	
	levels = backtrace(trace, 256);
	if (levels) {
		backtrace_symbols_fd(trace, levels, fd);
	}
#endif // HAVE_BACKTRACE
}
void kdClearDebugConfig()
{
    if (kDebug_data) {
        delete kDebug_data->config;
        kDebug_data->config = 0;
    }
}


// Needed for --enable-final
#ifdef NDEBUG
#define kdDebug kndDebug
#endif
