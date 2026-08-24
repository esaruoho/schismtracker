/* Ableton Link includes <WinSock2.h> with Windows' capitalisation. That
 * resolves fine on a case-insensitive filesystem, but when cross-compiling
 * for mingw from Linux the real header is <winsock2.h>. Forward to it.
 *
 * configure only puts this directory on the include path when the
 * capitalised spelling is genuinely missing, so a native Windows build
 * never sees these files. */
#ifndef SCHISM_MINGW_CASE_WINSOCK2_H_
#define SCHISM_MINGW_CASE_WINSOCK2_H_
#include <winsock2.h>
#endif
