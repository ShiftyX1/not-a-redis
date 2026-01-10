#pragma once

#include <stdint.h>
#include <unistd.h>

int32_t read_full(int connectionfd, void* buf, size_t len);
int32_t write_all(int connectionfd, const void* buf, size_t len);
void fd_set_nb(int fd);
