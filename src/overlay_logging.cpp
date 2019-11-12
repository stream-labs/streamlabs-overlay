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
#include <time.h>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <filesystem>

bool log_output_disabled = true;
bool log_output_old_stored = false;

namespace fs = std::filesystem;
std::ofstream log_output_file;

const std::string getTimeStamp() 
{
	const std::time_t t = std::time(nullptr);

	struct tm buf;
	localtime_s(&buf, &t);

	char mbstr[128]={0};
	std::strftime(mbstr, sizeof(mbstr), "%Y%m%d:%H%M%S.", &buf);
	unsigned __int64 now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	std::ostringstream ss;
	ss << mbstr << std::setw(3) << std::setfill('0') << now%1000;
	return ss.str();
}

void logging_start(std::string log_path)
{
    if (log_output_file.is_open())
    {
        log_output_disabled = false;
        return;
    }

    if( log_path.size() != 0)
    {
        std::error_code ec;
        if( !log_output_old_stored )
        {
            log_output_old_stored = true;
            try {
                fs::path log_file = log_path;
                fs::path log_file_old = log_file;
                log_file_old.replace_extension("log.old");
                fs::remove(log_file_old, ec);
                fs::rename(log_file, log_file_old, ec); 
                fs::remove(log_file, ec);
            } catch (...) {
            }   
        }   
        try {
            log_output_file.open( log_path, std::ios_base::out | std::ios_base::app);
            log_output_disabled = false;
        } catch (...) {
            log_output_disabled = true;
        }
    } else {
        log_output_disabled  = true;
    }
}

void logging_end()
{
    if( log_output_file.is_open() ) 
    {
        log_output_file.flush();
        log_output_disabled = true;
    }
}
