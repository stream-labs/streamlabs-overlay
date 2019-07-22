/******************************************************************************
    Copyright (C) 2016-2019 by Streamlabs (General Workings Inc)
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "overlay_logging.h"

#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip>

const std::string getTimeStamp() 
{
    const std::time_t t = std::time(nullptr);
    char mbstr[128]={0};
    std::strftime(mbstr, sizeof(mbstr), "%Y%m%d:%H%M%S.", std::localtime(&t));
    unsigned __int64 now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::ostringstream ss;
    ss << mbstr << std::setw(3) << std::setfill('0') << now%1000 <<  ": ";
    return ss.str();
}
