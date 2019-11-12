#pragma once

#include <iostream>
#include <fstream>
#include <string>


const std::string getTimeStamp();

extern std::ofstream log_output_file;
extern bool log_output_disabled;

#define log_info if (log_output_disabled || !log_output_file.is_open()) {} else log_output_file << "INF:" << getTimeStamp() <<  ": " 
#define log_debug if (log_output_disabled || !log_output_file.is_open()) {} else log_output_file << "DBG:" << getTimeStamp() <<  ": "
#define log_error if (log_output_disabled || !log_output_file.is_open()) {} else log_output_file << "ERR:" << getTimeStamp() <<  ": "

void logging_start(std::string log_path);
void logging_end();