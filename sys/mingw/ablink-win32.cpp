/* Windows wrapper around Link's C API translation unit.
 *
 * The Windows headers define `interface' as a macro for `struct'. Link
 * uses it as an ordinary identifier -- link_audio/Channels.hpp declares
 * `std::shared_ptr<Interface> interface' -- so the macro turns that into
 * a syntax error.
 *
 * Neither of the obvious fixes works. Undefining it in the
 * sys/mingw/case-compat shims misses asio, which reaches these headers by
 * their lowercase names. -DWIN32_LEAN_AND_MEAN does not help either:
 * windows.h defines the macro itself, on line 19, outside any such guard.
 *
 * mingw-w64 defines it in four places -- windows.h, rpc.h, combaseapi.h
 * and basetyps.h -- so undefining after only some of them just lets a
 * later include put it back. Pull in every Windows header this
 * translation unit can reach, drop the macro once they are all in, and
 * only then compile Link. Their include guards make asio's and Link's
 * later includes no-ops, so nothing redefines it.
 *
 * The list is every Windows header named by an #include under
 * link/include, link/extensions and the bundled asio. __has_include keeps
 * a toolchain that lacks one of them from breaking the build. */

#define SCHISM_TRY_INCLUDE(h) __has_include(h)

/* winsock2.h must precede windows.h, or windows.h pulls in the old
 * winsock.h and the two conflict. */
#if SCHISM_TRY_INCLUDE(<winsock2.h>)
# include <winsock2.h>
#endif
#if SCHISM_TRY_INCLUDE(<ws2tcpip.h>)
# include <ws2tcpip.h>
#endif
#if SCHISM_TRY_INCLUDE(<mswsock.h>)
# include <mswsock.h>
#endif
#if SCHISM_TRY_INCLUDE(<iphlpapi.h>)
# include <iphlpapi.h>
#endif
#if SCHISM_TRY_INCLUDE(<windows.h>)
# include <windows.h>
#endif

/* The other three that define the macro. */
#if SCHISM_TRY_INCLUDE(<basetyps.h>)
# include <basetyps.h>
#endif
#if SCHISM_TRY_INCLUDE(<rpc.h>)
# include <rpc.h>
#endif
#if SCHISM_TRY_INCLUDE(<combaseapi.h>)
# include <combaseapi.h>
#endif

/* Everything else asio and Link name, so none of them can drag one of the
 * above back in after the undef below. */
#if SCHISM_TRY_INCLUDE(<winapifamily.h>)
# include <winapifamily.h>
#endif
#if SCHISM_TRY_INCLUDE(<winerror.h>)
# include <winerror.h>
#endif
#if SCHISM_TRY_INCLUDE(<process.h>)
# include <process.h>
#endif
#if SCHISM_TRY_INCLUDE(<processthreadsapi.h>)
# include <processthreadsapi.h>
#endif
#if SCHISM_TRY_INCLUDE(<avrt.h>)
# include <avrt.h>
#endif
#if SCHISM_TRY_INCLUDE(<bcrypt.h>)
# include <bcrypt.h>
#endif
#if SCHISM_TRY_INCLUDE(<robuffer.h>)
# include <robuffer.h>
#endif

#undef SCHISM_TRY_INCLUDE

#undef interface

/* Relative to this file, so it resolves against $(top_srcdir) in a VPATH build. */
#include "../../link/extensions/abl_link/src/abl_link.cpp"
