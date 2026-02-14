#ifndef HELPERS_H
#define HELPERS_H
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "crow.h"

uint16_t get_server_port();
std::string get_timestamp();

#endif