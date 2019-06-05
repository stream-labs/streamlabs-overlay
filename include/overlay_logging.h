#pragma once

#include <iostream>

#ifdef _DEBUG
    const bool log_output_disabled = false;
#else
    const bool log_output_disabled = true;
#endif

#define log_cout if (log_output_disabled) {} else std::cout 
#define log_debug if (log_output_disabled) {} else std::cout 
#define log_error if (log_output_disabled) {} else std::cout 

