#if 1  // PLATFORM-SPECIFIC
#if _WIN32
# ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#  define _WINSOCK_DEPRECATED_NO_WARNINGS
# endif

# define WIN32_LEAN_AND_MEAN
# include <windows.h>
// # include <winsock2.h>
# include <ws2tcpip.h>
# include <mswsock.h>
# pragma comment(lib, "user32.lib")
# pragma comment(lib, "ws2_32.lib")
# pragma comment(lib, "libcmt")

typedef SOCKET Socket;
# define wb_socket_err() WSAGetLastError()

// needed on Linux for preventing SIGPIPE, but this isn't an issue for Windows
# define MSG_NOSIGNAL 0

#else // WIN32
# pragma comment(lib, "c")

# include <errno.h>
# include <sys/types.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <netinet/in.h>
# include <poll.h>
# include <sys/ioctl.h>
# include <sys/socket.h>
# include <unistd.h>


typedef int Socket;
# define INVALID_SOCKET (-1)
# define closesocket close
# define WSACleanup(...) 0
# define wb_socket_err() errno

// NOTE: can't do this the other way round because windows has conflicting errno.h values
# define WSAEWOULDBLOCK             EWOULDBLOCK
# define WSAEINPROGRESS             EINPROGRESS
# define WSAEALREADY                EALREADY
# define WSAENOTSOCK                ENOTSOCK
# define WSAEDESTADDRREQ            EDESTADDRREQ
# define WSAEMSGSIZE                EMSGSIZE
# define WSAEPROTOTYPE              EPROTOTYPE
# define WSAENOPROTOOPT             ENOPROTOOPT
# define WSAEPROTONOSUPPORT         EPROTONOSUPPORT
# define WSAESOCKTNOSUPPORT         ESOCKTNOSUPPORT
# define WSAEOPNOTSUPP              EOPNOTSUPP
# define WSAEPFNOSUPPORT            EPFNOSUPPORT
# define WSAEAFNOSUPPORT            EAFNOSUPPORT
# define WSAEADDRINUSE              EADDRINUSE
# define WSAEADDRNOTAVAIL           EADDRNOTAVAIL
# define WSAENETDOWN                ENETDOWN
# define WSAENETUNREACH             ENETUNREACH
# define WSAENETRESET               ENETRESET
# define WSAECONNABORTED            ECONNABORTED
# define WSAECONNRESET              ECONNRESET
# define WSAENOBUFS                 ENOBUFS
# define WSAEISCONN                 EISCONN
# define WSAENOTCONN                ENOTCONN
# define WSAESHUTDOWN               ESHUTDOWN
# define WSAETOOMANYREFS            ETOOMANYREFS
# define WSAETIMEDOUT               ETIMEDOUT
# define WSAECONNREFUSED            ECONNREFUSED
# define WSAELOOP                   ELOOP
# define WSAENAMETOOLONG            ENAMETOOLONG
# define WSAEHOSTDOWN               EHOSTDOWN
# define WSAEHOSTUNREACH            EHOSTUNREACH
# define WSAENOTEMPTY               ENOTEMPTY
# define WSAEPROCLIM                EPROCLIM
# define WSAEUSERS                  EUSERS
# define WSAEDQUOT                  EDQUOT
# define WSAESTALE                  ESTALE
# define WSAEREMOTE                 EREMOTE

#endif //_WIN32
#endif // PLATFORM-SPECIFIC


#include "plugin.h"
#define AHD_IMPLEMENTATION
#include "airhead.h"

#define WB_S(str) str, (sizeof(str)-1)

#ifndef  WB_PORT
# define WB_PORT 19013
#endif //WB_PORT

typedef enum { SOCK_none, SOCK_startup, SOCK_socket, SOCK_connect } WBSockStatus;

struct WBPluginGlobals {
    Socket       sock;
    bool         debug;
    WBSockStatus sock_status;

    char   *buf;
} g_wb = { INVALID_SOCKET, false };


static inline Size wb_str_len(Char const *str)
{
    Size result = 0;
    for (Char const *at = str; *at; ++at){ ++result; }
    return result;
}



static void wb_editor_message(Char const *title, Char const *message, Size message_len);
static void inline wb_errorf(bool is_noisy, Char const *title, Char const *fmt, ...)
{
    if (is_noisy)
    {
        va_list args; va_start(args, fmt);
        arr_clear(g_wb.buf);
        arr_vprintf(&g_wb.buf, fmt, args);
        va_end(args);
        wb_editor_message(title, g_wb.buf, arr_len(g_wb.buf)-1);
    }
}
static inline void wb_error(bool is_noisy, Char const *title, Char const *message, int message_len)
{   if (is_noisy) wb_editor_message(title, message, message_len);   }


static inline bool
wb_is_connected()
{   return g_wb.sock_status == SOCK_connect && g_wb.sock != INVALID_SOCKET;   }


static bool
wb_connect_socket(bool is_noisy)
{
    switch (g_wb.sock_status)
    {
        case SOCK_none:
        {
            #if _WIN32
            WSADATA dummy;
            int startup_err = WSAStartup(MAKEWORD(2,2), &dummy);
            if (startup_err != 0)
            {   wb_errorf(is_noisy, "WhiteBox Socket Error", "Winsock 2.2 not supported on this platform. Cannot continue. (%d)", wb_socket_err()); break;   }
            #endif//_WIN32
            g_wb.sock_status = SOCK_startup;
        } // fallthrough

        case SOCK_startup:
        {
            g_wb.sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (g_wb.sock == INVALID_SOCKET)
            {   wb_errorf(is_noisy, "WhiteBox Socket Error", "Could not create socket. (%d)", wb_socket_err()); break;   }
            g_wb.sock_status = SOCK_socket;
        } // fallthrough


        case SOCK_socket:
        {
            struct sockaddr_in addr = {0}; {
                addr.sin_family = AF_INET;
                addr.sin_port   = htons(WB_PORT);
                // loopback only includes this machine; could change to INET_ANY for larger network,
                // but this starts to involve firewalls etc, so is slower
                addr.sin_addr.s_addr = htonl(0x7f000001); // 127.0.0.1, i.e. localhost/loopback
            }
            int conn_err = connect(g_wb.sock, (struct sockaddr *)&addr, sizeof(addr));
            int sock_err = wb_socket_err();
            if ((conn_err || g_wb.sock == INVALID_SOCKET) &&
                sock_err != WSAEISCONN) // "already connected"
            {   wb_errorf(is_noisy, "WhiteBox Socket Error", "Could not connect. Is WhiteBox running? (%d)", sock_err); break;   }
            g_wb.sock_status = SOCK_connect;
        }

        case SOCK_connect:; // done
    }

    return g_wb.sock_status == SOCK_connect;
}
static inline bool wb_connect(bool is_noisy) { return wb_connect_socket(is_noisy); }


static void
arr_print_escaped_json(Char **arr, Char const *str, Size str_len)
{
    for (Size i = 0; i < str_len && str[i]; ++i)
    { // escape JSON strings as needed
        char const *s = NULL;
        switch (str[i])
        {
            case '\b': s = "\\b";  break;
            case '\f': s = "\\f";  break;
            case '\n': s = "\\n";  break;
            case '\r': s = "\\r";  break;
            case '\t': s = "\\t";  break;
            case '"':  s = "\\\""; break;
            case '\\': s = "\\\\"; break;
            default: arr_putc(str[i], arr); continue;
        }
        arr_puts_n(s, 2, arr);
    }
}


static inline bool
wb_close_socket(bool is_noisy)
{
    int close_err = closesocket(g_wb.sock);
    g_wb.sock = INVALID_SOCKET;
    if (close_err)
    {   wb_errorf(is_noisy, "WhiteBox Socket Error", "Couldn't close socket. (%d)", wb_socket_err());   }
    g_wb.sock_status = SOCK_startup;
    return (bool)close_err;
}

static bool
wb_disconnect_socket(bool is_noisy)
{
    switch (g_wb.sock_status)
    {
        case SOCK_connect: // fallthrough // shutdown?
        case SOCK_socket: if (wb_close_socket(is_noisy)) { break; } //fallthrough
        case SOCK_startup: {
            if (WSACleanup())
            {   wb_errorf(is_noisy, "WhiteBox Socket Error", WB_S("Socket cleanup failed. (%d)"), wb_socket_err()); break;   }
            g_wb.sock_status = SOCK_none;
        } //fallthrough

        case SOCK_none:; // done
    }
    return g_wb.sock_status == SOCK_none;
}
static inline bool wb_disconnect(bool is_noisy) { return wb_disconnect_socket(is_noisy); }


static bool
wb_send_plugin_data(Selection sel, Char const *path, Size path_len, Char const *editor, DirtyState dirty_state, int ack_pkt = 0)
{
    bool result = 0;

    if (! path)
    {   wb_error(1, "WhiteBox error", WB_S("Missing file path."));   }
    else if (path[0] && wb_is_connected())
    {
        arr_clear(g_wb.buf);
        arr_printf(&g_wb.buf,
            "{\n"
            " editor: \"%s\",\n"
            " path: \"", editor);
        arr_print_escaped_json(&g_wb.buf, path, path_len);
        arr_printf(&g_wb.buf, "\",\n"
            " selection: [\n"
            " {line:%zu, column:%zu},\n"
            " {line:%zu, column:%zu},\n"
            " ],\n"
            " dirty:[%s],\n"
            " ack_pkt: %d,\n"
            #if 0
            //todo generate supports string from optional Supports object
            " supports:\n"
            " {\n"
            "  receiving_data: true,\n"
            "  setting_cursor: true,\n"
            " }"
            #endif
            "}",
            sel.current.line, sel.current.clmn, sel.initial.line, sel.initial.clmn,
            (dirty_state & DIRTY_unsaved ? "\"unsaved\"" : ""), ack_pkt);

        if (g_wb.debug) { wb_editor_message("WhiteBox sending data", g_wb.buf, arr_len(g_wb.buf)); }

        // TODO IMPORTANT: don't freeze if messages don't get immediately responded to - NONBLOCKING?
        S8  send_err = send(g_wb.sock, g_wb.buf, (int)arr_len(g_wb.buf), MSG_NOSIGNAL);
        int sock_err = wb_socket_err();
        result       = send_err != -1;

        if (g_wb.debug && 0) { wb_errorf(true, "WhiteBox sent data", "result: %zd", send_err); }

        if (! result) switch (sock_err)
        {
            case EPIPE:
            case WSAECONNRESET: {
                wb_close_socket(1);
                if (wb_connect(false))
                {   wb_errorf(true, "WhiteBox", "Connection reopened");   }
                else
                {   wb_errorf(true, "WhiteBox", "Connection closed");   }
            } break;

            default: wb_errorf(true, "WhiteBox Socket Error", "Error sending WhiteBox data (%d)", sock_err);
        }
    }
    return result;
}


#undef wb_socket_err
#undef WB_S
#undef WB_PORT

#if _WIN32
# undef WIN32_LEAN_AND_MEAN
#else
# undef closesocket
# undef WSACleanup
# undef INVALID_SOCKET

# undef WSAEWOULDBLOCK
# undef WSAEINPROGRESS
# undef WSAEALREADY
# undef WSAENOTSOCK
# undef WSAEDESTADDRREQ
# undef WSAEMSGSIZE
# undef WSAEPROTOTYPE
# undef WSAENOPROTOOPT
# undef WSAEPROTONOSUPPORT
# undef WSAESOCKTNOSUPPORT
# undef WSAEOPNOTSUPP
# undef WSAEPFNOSUPPORT
# undef WSAEAFNOSUPPORT
# undef WSAEADDRINUSE
# undef WSAEADDRNOTAVAIL
# undef WSAENETDOWN
# undef WSAENETUNREACH
# undef WSAENETRESET
# undef WSAECONNABORTED
# undef WSAECONNRESET
# undef WSAENOBUFS
# undef WSAEISCONN
# undef WSAENOTCONN
# undef WSAESHUTDOWN
# undef WSAETOOMANYREFS
# undef WSAETIMEDOUT
# undef WSAECONNREFUSED
# undef WSAELOOP
# undef WSAENAMETOOLONG
# undef WSAEHOSTDOWN
# undef WSAEHOSTUNREACH
# undef WSAENOTEMPTY
# undef WSAEPROCLIM
# undef WSAEUSERS
# undef WSAEDQUOT
# undef WSAESTALE
# undef WSAEREMOTE
#endif
