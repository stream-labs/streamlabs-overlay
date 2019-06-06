#pragma once

#include <iostream>
#include <string>

#ifdef _DEBUG
    const bool log_output_disabled = false;
#else
    const bool log_output_disabled = true;
#endif

const std::string getTimeStamp();

#define log_cout if (log_output_disabled) {} else std::cout << getTimeStamp() 
#define log_debug if (log_output_disabled) {} else std::cout << getTimeStamp() 
#define log_error if (log_output_disabled) {} else std::cout << getTimeStamp() 

