#ifndef _COMMON_H_
#define _COMMON_H_

enum class status {
    ok,
    error,
    send_error,
    receive_error,
    closed_connection,
    header_error,
    body_error
};

#endif
