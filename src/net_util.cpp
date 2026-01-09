#include "net_util.hpp"
#include <assert.h>
#include <string>


int32_t read_full(int connectionfd, void* buf, size_t len) {
    auto buffer = static_cast<char*>(buf);
    while (len > 0) {
        ssize_t rv = ::read(connectionfd, buffer, len);
        if (rv == 0) {
            return -1;
        }
        if (rv < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        buffer += rv;
        len -= static_cast<size_t>(rv);
    }
    return 0;
}

int32_t write_all(int connectionfd, const void* buf, size_t len) {
    auto* buffer = static_cast<const char*>(buf);
    while (len > 0) {
        ssize_t rv = ::write(connectionfd, buffer, len);
        if (rv <= 0) {
            if (rv < 0 && errno == EINTR) continue;
            return -1;
        }
        buffer += rv;
        len -= static_cast<size_t>(rv);
    }
    return 0;
}
