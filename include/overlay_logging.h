#pragma once

#include <iostream>

#ifdef _DEBUG
    const bool debug_disabled = false;
#else
    const bool debug_disabled = true;
#endif

#define log_cout if (debug_disabled) {} else std::cout 

