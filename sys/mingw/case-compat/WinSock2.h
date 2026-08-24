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

/* windows.h (reached directly or via winsock2.h) defines `interface' as a
 * macro for `struct'. Link uses it as an ordinary parameter name --
 * link_audio/Channels.hpp takes a `std::shared_ptr<Interface> interface' --
 * so leaving the macro defined turns that into a syntax error. Nothing here
 * wants the macro; drop it. Undone in every shim rather than just this one,
 * because whichever gets included first is what pulls windows.h in. */
#undef interface

#endif
