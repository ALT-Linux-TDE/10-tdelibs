/* $Xorg: ICEproto.h,v 1.4 2000/08/17 19:44:11 cpqbld Exp $ */
/******************************************************************************


Copyright 1993, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

Author: Ralph Mor, X Consortium
******************************************************************************/

#ifndef _ICEPROTO_H_
#define _ICEPROTO_H_

#include "config.h"
#if defined TQ_WS_X11 && ! defined K_WS_QTONLY
#include <X11/Xmd.h>
#else
#if defined(__alpha__) || defined(__ia64__) || defined(__s390x__)
typedef unsigned int CARD32;
#else
typedef unsigned long CARD32;
#endif
typedef unsigned short CARD16;
typedef unsigned char CARD8;
#include <unistd.h>
#endif

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	data[2];
    CARD32	length :32;
} iceMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD16	errorClass :16;
    CARD32	length :32;
    CARD8	offendingMinorOpcode;
    CARD8	severity;
    CARD16	unused :16;
    CARD32	offendingSequenceNum :32;
    /* n	varying values */
    /* p	p = pad (n, 8) */
} iceErrorMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	byteOrder;
    CARD8	unused;
    CARD32	length :32;
} iceByteOrderMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	versionCount;
    CARD8	authCount;
    CARD32	length :32;
    CARD8	mustAuthenticate;
    CARD8	unused[7];
    /* i	STRING		vendor */
    /* j	STRING		release */
    /* k	LIST of STRING	authentication-protocol-names */
    /* m	LIST of VERSION version-list */
    /* p	p = pad (i+j+k+m, 8) */
} iceConnectionSetupMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	authIndex;
    CARD8	unused1;
    CARD32	length :32;
    CARD16	authDataLength :16;
    CARD8	unused2[6];
    /* n	varying data */
    /* p	p = pad (n, 8) */
} iceAuthRequiredMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	unused1[2];
    CARD32	length :32;
    CARD16	authDataLength :16;
    CARD8	unused2[6];
    /* n	varying data */
    /* p	p = pad (n, 8) */
} iceAuthReplyMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	unused1[2];
    CARD32	length :32;
    CARD16	authDataLength :16;
    CARD8	unused2[6];
    /* n	varying data */
    /* p	p = pad (n, 8) */
} iceAuthNextPhaseMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	versionIndex;
    CARD8	unused;
    CARD32	length :32;
    /* i	STRING		vendor */
    /* j	STRING		release */
    /* p	p = pad (i+j, 8) */
} iceConnectionReplyMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	protocolOpcode;
    CARD8	mustAuthenticate;
    CARD32	length :32;
    CARD8	versionCount;
    CARD8	authCount;
    CARD8	unused[6];
    /* i	STRING		protocol-name */
    /* j	STRING		vendor */
    /* k	STRING		release */
    /* m	LIST of STRING	authentication-protocol-names */
    /* n	LIST of VERSION version-list */
    /* p        p = pad (i+j+k+m+n, 8) */
} iceProtocolSetupMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	versionIndex;
    CARD8	protocolOpcode;
    CARD32	length :32;
    /* i	STRING		vendor */
    /* j	STRING		release */
    /* p	p = pad (i+j, 8) */
} iceProtocolReplyMsg;

typedef iceMsg  icePingMsg;
typedef iceMsg  icePingReplyMsg;
typedef iceMsg  iceWantToCloseMsg;
typedef iceMsg  iceNoCloseMsg;


/*
 * SIZEOF values.  These better be multiples of 8.
 */

#define sz_iceMsg			8
#define sz_iceErrorMsg			16
#define sz_iceByteOrderMsg		8
#define sz_iceConnectionSetupMsg        16
#define sz_iceAuthRequiredMsg		16
#define sz_iceAuthReplyMsg		16
#define sz_iceAuthNextPhaseMsg		16
#define sz_iceConnectionReplyMsg	8
#define sz_iceProtocolSetupMsg		16
#define sz_iceProtocolReplyMsg		8
#define sz_icePingMsg			8
#define sz_icePingReplyMsg		8
#define sz_iceWantToCloseMsg		8
#define sz_iceNoCloseMsg		8

#endif /* _ICEPROTO_H_ */
