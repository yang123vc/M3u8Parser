/**
 * author: oujiangping@dvt.dvt.com
 * date: 2016/06/15
 * describe: base class for playlist
 */

#ifndef __M3U8BASE_H__
#define __M3U8BASE_H__

#include "StringHelper.h"

class M3U8Base {
    protected:
        int version;
    public:
        M3U8Base():
            version(-1){
            };

        int getM3UVersion() {return version;};

        void setM3UVersion(int v) {version = v;};

        static bool parseM3UVersion(const std::string &line, int *x) {
            int colonPos = line.find(":");
            if(colonPos < 0) {
                return false; 
            }
            bool ret = StringHelper::parseInt32(line.c_str() + colonPos + 1, x); 
            return ret;
        }
};

#endif
