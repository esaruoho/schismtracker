/* Ableton Link includes <Windows.h> with Windows' capitalisation. That
 * resolves fine on a case-insensitive filesystem, but when cross-compiling
 * for mingw from Linux the real header is <windows.h>. Forward to it.
 *
 * configure only puts this directory on the include path when the
 * capitalised spelling is genuinely missing, so a native Windows build
 * never sees these files. */
#ifndef SCHISM_MINGW_CASE_WINDOWS_H_
#define SCHISM_MINGW_CASE_WINDOWS_H_
#include <windows.h>
#endif
