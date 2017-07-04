// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


/*
 * log_formater.cpp
 *
 *  Created on: 2013-3-8
 *      Author: yerungui
 */


#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include <algorithm>

#include "mars/comm/xlogger/xloggerbase.h"
#include "mars/comm/xlogger/loginfo_extract.h"
#include "mars/comm/ptrbuffer.h"

#ifdef _WIN32
#define PRIdMAX "lld"
#define snprintf _snprintf
#else
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#endif

void log_formater(const XLoggerInfo* _info, const char* _logbody, PtrBuffer& _log) {
    assert((unsigned int)_log.Pos() == _log.Length());

    static int error_count = 0;
    static int error_size = 0;

    if (_log.MaxLength() <= _log.Length() + 5 * 1024) {  // allowd len(_log) <= 11K(16K - 5K)
        ++error_count;
        error_size = (int)strnlen(_logbody, 1024 * 1024);

        if (_log.MaxLength() >= _log.Length() + 128) {
            int ret = snprintf((char*)_log.PosPtr(), 1024, "[F]log_size <= 5*1024, err(%d, %d)\n", error_count, error_size);  // **CPPLINT SKIP**
            _log.Length(_log.Pos() + ret, _log.Length() + ret);
            _log.Write("");

            error_count = 0;
            error_size = 0;
        }

        assert(false);
        return;
    }

    if (NULL != _info) {
        char temp_time[64] = {0};

        if (0 != _info->timeval.tv_sec) {
            time_t sec = _info->timeval.tv_sec;
            tm tm = *localtime((const time_t*)&sec);
            // yyyy-MM-dd HH:mm:ss.SSS +Z.
            snprintf(temp_time, sizeof(temp_time), "%d-%02d-%02d %02d:%02d:%02d.%.3d %s%02d:00", 1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, _info->timeval.tv_usec / 1000, (tm.tm_gmtoff > 0 ? "+" : "-"), (int)(tm.tm_gmtoff / 3600.0));
        }
        
        int ret = snprintf((char*)_log.PosPtr(), 1024, "[%s] ***** ", temp_time);

        assert(0 <= ret);
        _log.Length(_log.Pos() + ret, _log.Length() + ret);

        assert((unsigned int)_log.Pos() == _log.Length());
    }

    if (NULL != _logbody) {

        size_t bodylen =  _log.MaxLength() - _log.Length() > 130 ? _log.MaxLength() - _log.Length() - 130 : 0;
        bodylen = bodylen > 0xFFFFU ? 0xFFFFU : bodylen;
        bodylen = strnlen(_logbody, bodylen);
        bodylen = bodylen > 0xFFFFU ? 0xFFFFU : bodylen;
        _log.Write(_logbody, bodylen);
    } else {
        _log.Write("error!! NULL==_logbody");
    }

    char nextline = '\n';

    if (*((char*)_log.PosPtr() - 1) != nextline) _log.Write(&nextline, 1);
}

