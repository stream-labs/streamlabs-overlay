#include "overlay_logging.h"

#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip>

const std::string getTimeStamp() 
{
    std::time_t t = std::time(nullptr);
    char mbstr[128]={0};
    std::strftime(mbstr, sizeof(mbstr), "%Y%m%d:%H%M%S.", std::localtime(&t));
    unsigned __int64 now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::ostringstream ss;
    ss << mbstr << std::setw(3) << std::setfill('0') << now%1000 <<  ": ";
    return ss.str();
}
