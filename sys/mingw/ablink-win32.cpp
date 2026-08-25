/* Windows wrapper around Link's C API translation unit.
 *
 * windows.h (and friends) define `interface' as a macro for `struct'.
 * Link uses it as an ordinary identifier -- link_audio/Channels.hpp
 * declares `std::shared_ptr<Interface> interface' -- so the macro turns
 * that into a syntax error.
 *
 * Undefining it in the sys/mingw/case-compat shims does not work, because
 * asio reaches the Windows headers through their lowercase names and so
 * never passes through those shims. -DWIN32_LEAN_AND_MEAN does not work
 * either: it stops windows.h pulling in ole2.h/objbase.h, but something
 * else in this include graph still defines the macro.
 *
 * So take ordering out of the picture. Pull in every Windows header this
 * translation unit can reach FIRST, drop the macro once they are all in,
 * and only then compile Link. Their include guards mean the later
 * includes from asio and Link are no-ops, so nothing puts it back. */

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <windows.h>

#undef interface

/* Relative to this file, so it resolves against $(top_srcdir) in a VPATH build. */
#include "../../link/extensions/abl_link/src/abl_link.cpp"
