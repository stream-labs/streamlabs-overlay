#pragma once

#include <iostream>
//extern std::ostream & log_cout;

#ifdef _DEBUG
    const bool debug_disabled = false;
    //std::ostream & log_cout = std::cout;
#else
    const bool debug_disabled = true;
#endif

#define log_cout if (debug_disabled) {} else std::cout 

