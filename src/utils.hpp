#pragma once

#include <netinet/in.h>
#include <string>

void stop_program(int signum);
void print_error(const std::string& msg);
void print_logo();
