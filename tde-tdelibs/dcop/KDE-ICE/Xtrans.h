/* $TOG: Xtrans.h /main/31 1998/02/06 15:55:22 kaleb $ */
/*

Copyright 1993, 1994, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/lib/xtrans/Xtrans.h,v 3.14 1999/12/13 02:52:54 robin Exp $ */

/* Copyright 1993, 1994 NCR Corporation - Dayton, Ohio, USA
 *
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name NCR not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  NCR makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NCR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NCR BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _XTRANS_H_
#define _XTRANS_H_

#include "config.h"
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
#include <X11/Xfuncproto.h>
#include <X11/Xos.h>
#else
#include <KDE-ICE/Xfuncproto.h>
#include <fcntl.h>
#endif

/*
 * Set the functions names according to where this code is being compiled.
 */

#ifdef X11_t
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _X11Trans##func
#else
#define TRANS(func) _X11Trans/**/func
#endif
#ifdef XTRANSDEBUG
static char* __xtransname = "_X11Trans";
#endif
#endif /* X11_t */

#ifdef XSERV_t
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _XSERVTrans##func
#else
#define TRANS(func) _XSERVTrans/**/func
#endif
#ifdef XTRANSDEBUG
static char* __xtransname = "_XSERVTrans";
#endif
#define X11_t
#endif /* X11_t */

#ifdef XIM_t
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _XimXTrans##func
#else
#define TRANS(func) _XimXTrans/**/func
#endif
#ifdef XTRANSDEBUG
static char* __xtransname = "_XimTrans";
#endif
#endif /* XIM_t */

#ifdef FS_t
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _FSTrans##func
#else
#define TRANS(func) _FSTrans/**/func
#endif
#ifdef XTRANSDEBUG
static char* __xtransname = "_FSTrans";
#endif
#endif /* FS_t */

#ifdef FONT_t
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _FontTrans##func
#else
#define TRANS(func) _FontTrans/**/func
#endif
#ifdef XTRANSDEBUG
static char* __xtransname = "_FontTrans";
#endif
#endif /* FONT_t */

#ifdef ICE_t
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _kde_IceTrans##func
#else
#define TRANS(func) _kde_IceTrans/**/func
#endif
#ifdef XTRANSDEBUG
static char* __xtransname = (char *)"_kde_IceTrans";
#endif
#endif /* ICE_t */

#ifdef TEST_t
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _TESTTrans##func
#else
#define TRANS(func) _TESTTrans/**/func
#endif
#ifdef XTRANSDEBUG
static char* __xtransname = "_TESTTrans";
#endif
#endif /* TEST_t */

#ifdef LBXPROXY_t
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _LBXPROXYTrans##func
#else
#define TRANS(func) _LBXPROXYTrans/**/func
#endif
#define X11_t		/* The server defines this - so should the LBX proxy */
#ifdef XTRANSDEBUG
static char* __xtransname = "_LBXPROXYTrans";
#endif
#endif /* LBXPROXY_t */

#if !defined(TRANS)
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _XTrans##func
#else
#define TRANS(func) _XTrans/**/func
#endif
#ifdef XTRANSDEBUG
static char* __xtransname = "_XTrans";
#endif
#endif /* !TRANS */


/*
 * Create a single address structure that can be used wherever
 * an address structure is needed. struct sockaddr is not big enough
 * to hold a sockadd_un, so we create this definition to have a single
 * structure that is big enough for all the structures we might need.
 *
 * This structure needs to be independent of the socket/TLI interface used.
 */

#define XTRANS_MAX_ADDR_LEN	128	/* large enough to hold sun_path */

typedef	struct {
    unsigned char	addr[XTRANS_MAX_ADDR_LEN];
} Xtransaddr;


#ifdef LONG64
typedef int BytesReadable_t;
#else
typedef long BytesReadable_t;
#endif


#if defined(_WIN32) || (defined(USG) && !defined(CRAY) && !defined(umips) && !defined(MOTOROLA) && !defined(uniosu) && !defined(__sxg__)) || defined(MINIX)

/*
 *      TRANS(Readv) and TRANS(Writev) use struct iovec, normally found
 *      in Berkeley systems in <sys/uio.h>.  See the readv(2) and writev(2)
 *      manual pages for details.
 */

struct iovec {
    caddr_t iov_base;
    int iov_len;
};

#else
#ifndef Lynx
#include <sys/uio.h>
#else
#include <uio.h>
#endif
#endif

typedef struct _XtransConnInfo *XtransConnInfo;


/*
 * Transport Option definitions
 */

#define TRANS_NONBLOCKING	1
#define	TRANS_CLOSEONEXEC	2


/*
 * Return values of Connect (0 is success)
 */

#define TRANS_CONNECT_FAILED 	-1
#define TRANS_TRY_CONNECT_AGAIN -2
#define TRANS_IN_PROGRESS	-3


/*
 * Return values of CreateListener (0 is success)
 */

#define TRANS_CREATE_LISTENER_FAILED 	-1
#define TRANS_ADDR_IN_USE		-2


/*
 * Return values of Accept (0 is success)
 */

#define TRANS_ACCEPT_BAD_MALLOC			-1
#define TRANS_ACCEPT_FAILED 			-2
#define TRANS_ACCEPT_MISC_ERROR			-3


/*
 * ResetListener return values
 */

#define TRANS_RESET_NOOP	1
#define TRANS_RESET_NEW_FD	2
#define TRANS_RESET_FAILURE	3


/*
 * Function prototypes for the exposed interface
 */

void TRANS(FreeConnInfo) (
    XtransConnInfo 	/* ciptr */
);

#ifdef TRANS_CLIENT

XtransConnInfo TRANS(OpenCOTSClient)(
    char *		/* address */
);

#endif /* TRANS_CLIENT */

#ifdef TRANS_SERVER

XtransConnInfo TRANS(OpenCOTSServer)(
    char *		/* address */
);

#endif /* TRANS_SERVER */

#ifdef TRANS_CLIENT

XtransConnInfo TRANS(OpenCLTSClient)(
    char *		/* address */
);

#endif /* TRANS_CLIENT */

#ifdef TRANS_SERVER

XtransConnInfo TRANS(OpenCLTSServer)(
    char *		/* address */
);

#endif /* TRANS_SERVER */

#ifdef TRANS_REOPEN

XtransConnInfo TRANS(ReopenCOTSServer)(
    int,		/* trans_id */
    int,		/* fd */
    char *		/* port */
);

XtransConnInfo TRANS(ReopenCLTSServer)(
    int,		/* trans_id */
    int,		/* fd */
    char *		/* port */
);

int TRANS(GetReopenInfo)(
    XtransConnInfo,	/* ciptr */
    int *,		/* trans_id */
    int *,		/* fd */
    char **		/* port */
);

#endif /* TRANS_REOPEN */


int TRANS(SetOption)(
    XtransConnInfo,	/* ciptr */
    int,		/* option */
    int			/* arg */
);

#ifdef TRANS_SERVER

int TRANS(CreateListener)(
    XtransConnInfo,	/* ciptr */
    char *		/* port */
);

int TRANS(NoListen) (
    char*               /* protocol*/
);

int TRANS(ResetListener)(
    XtransConnInfo	/* ciptr */
);

XtransConnInfo TRANS(Accept)(
    XtransConnInfo,	/* ciptr */
    int *		/* status */
);

#endif /* TRANS_SERVER */

#ifdef TRANS_CLIENT

int TRANS(Connect)(
    XtransConnInfo,	/* ciptr */
    char *		/* address */
);

#endif /* TRANS_CLIENT */

int TRANS(BytesReadable)(
    XtransConnInfo,	/* ciptr */
    BytesReadable_t *	/* pend */
);

int TRANS(Read)(
    XtransConnInfo,	/* ciptr */
    char *,		/* buf */
    int			/* size */
);

int TRANS(Write)(
    XtransConnInfo,	/* ciptr */
    char *,		/* buf */
    int			/* size */
);

int TRANS(Readv)(
    XtransConnInfo,	/* ciptr */
    struct iovec *,	/* buf */
    int			/* size */
);

int TRANS(Writev)(
    XtransConnInfo,	/* ciptr */
    struct iovec *,	/* buf */
    int			/* size */
);

int TRANS(Disconnect)(
    XtransConnInfo	/* ciptr */
);

int TRANS(Close)(
    XtransConnInfo	/* ciptr */
);

int TRANS(CloseForCloning)(
    XtransConnInfo	/* ciptr */
);

int TRANS(IsLocal)(
    XtransConnInfo	/* ciptr */
);

int TRANS(GetMyAddr)(
    XtransConnInfo,	/* ciptr */
    int *,		/* familyp */
    int *,		/* addrlenp */
    Xtransaddr **	/* addrp */
);

int TRANS(GetPeerAddr)(
    XtransConnInfo,	/* ciptr */
    int *,		/* familyp */
    int *,		/* addrlenp */
    Xtransaddr **	/* addrp */
);

int TRANS(GetConnectionNumber)(
    XtransConnInfo	/* ciptr */
);

#ifdef TRANS_SERVER

int TRANS(MakeAllCOTSServerListeners)(
    char *,		/* port */
    int *,		/* partial */
    int *,		/* count_ret */
    XtransConnInfo **	/* ciptrs_ret */
);

int TRANS(MakeAllCLTSServerListeners)(
    char *,		/* port */
    int *,		/* partial */
    int *,		/* count_ret */
    XtransConnInfo **	/* ciptrs_ret */
);

#endif /* TRANS_SERVER */


/*
 * Function Prototypes for Utility Functions.
 */

#ifdef X11_t

int TRANS(ConvertAddress)(
    int *,		/* familyp */
    int *,		/* addrlenp */
    Xtransaddr **	/* addrp */
);

#endif /* X11_t */

#if defined(ICE_t)

char *
TRANS(GetMyNetworkId)(
    XtransConnInfo	/* ciptr */
);

char *
TRANS(GetPeerNetworkId)(
    XtransConnInfo	/* ciptr */
);

#endif /* ICE_t */

int
TRANS(GetHostname) (
    char *	/* buf */,
    int 	/* maxlen */
);

#if defined(_WIN32) && (defined(TCPCONN) || defined(DNETCONN))
int TRANS(WSAStartup)();
#endif

#endif /* _XTRANS_H_ */
