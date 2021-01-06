/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2019 Lennart Andersson.
 *
 * This file is part of OPS (Open Publish Subscribe).
 *
 * OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
 */
#include "OPSTypeDefs.h"
#include "TimeHelper.h"

#ifndef REPLACE_TRANSPORT_LAYER

#include <chrono>
#include <thread>
#include <ctime>

namespace ops
{
    ///Returns the current time as a number of milliseconds since Epoch 1970-01-01.
    int64_t TimeHelper::currentTimeMillis() noexcept
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    void TimeHelper::sleep(const std::chrono::milliseconds& sleep_duration)
    {
        std::this_thread::sleep_for(sleep_duration);
    }

    void TimeHelper::sleep(const std::chrono::seconds& sleep_duration)
    {
        std::this_thread::sleep_for(sleep_duration);
    }

    ///Sleeps the given number of milliseconds (millis).
    void TimeHelper::sleep(const int64_t millis)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(millis));
    }

	///Returns current system time as a string to be used as user output, file names etc...
    std::string TimeHelper::getTimeToString()
    {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
        const std::time_t t = std::time(nullptr);
        char mbstr[100];
        if (std::strftime(mbstr, sizeof(mbstr), "%Y-%m-%d %H-%M-%S", std::localtime(&t)) > 0) {
            return mbstr;
        }
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
        return "";
    }

	///Returns the current time as a number of milliseconds since Epoch 1970-01-01.
    int64_t TimeHelper::getEpochTime() noexcept
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

}
#endif
