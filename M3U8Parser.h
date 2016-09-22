#include<iostream>
#include <map>
#include <list>
#include <vector>

#include "StringHelper.h"
#include "MediaSegment.h"
#include "HLSError.h"
#include "HLSCommon.h"
#include "M3U8Base.h"
#include "M3UMedia.h"

enum {
    M3U8_TYPE_UNKNOWN = 0,
    M3U8_TYPE_MASTER = 1,
    M3U8_TYPE_MEDIA = 2
};

enum {
    TAG_TYPE_EXTM3U = 1,
    TAG_TYPE_X_VERSION = 2,
};

class M3U8Parser {
    private:
        int playListType;
        bool isM3U8File;
        std::vector<MediaSegment*> mMediaSegmentVector;
        M3U8Base *m3u8Base;
        M3UMedia *m3uMedia;
        int m3u8Version;
    public:
        M3U8Parser():
            playListType(M3U8_TYPE_UNKNOWN),
            m3u8Version(-1),
            m3u8Base(NULL),
            m3uMedia(NULL),
            isM3U8File(false){
            }

        M3UMedia* getM3uMedia() {
            return m3uMedia;
        }

        int parser(const char* data, int size) {
            int offset = 0;
            int lineNo = -1;
            MediaSegment *curMediaSegment = NULL;
            while(offset < size) {
                //std::cout<<"offset is "<<offset<<" size is "<<size<<std::endl;
                std::string line = StringHelper::getLine(data, &offset, size);
                lineNo++;

                //the first line of m3u file must be "#EXTM3U"
                if(lineNo == 0) {
                    if(line == EXTM3U) {
                        isM3U8File = true;
                        std::cout<<"this is a m3u8 file\n";
                    } else {
                        return ERROR_NOT_M3U8_FILE;
                    }
                } else if(lineNo >= 1) {
                    //std::cout<<"line "<<line<<std::endl;
                    if(StringHelper::startWith(line, EXT_X_VERSION)) {
                        int v = -1;
                        if(M3U8Base::parseM3UVersion(line, &v)) {
                            m3u8Version = v;
                            std::cout<<"m3u8 version is "<<m3u8Version<<std::endl;
                        }
                        continue;
                    }

                    //parse master m3u8
                    if(StringHelper::startWith(line, EXT_X_STREAM_INF)) {
                        if(playListType == M3U8_TYPE_MASTER) {
                            return ERROR_MALFORMED;
                        }
                        playListType = M3U8_TYPE_MASTER;
                    }
                    if(!StringHelper::startWith(line, EXT_X_MEDIA_SEQUENCE) && StringHelper::startWith(line, EXT_X_MEDIA)) {
                        if(playListType == M3U8_TYPE_MASTER) {
                            return ERROR_MALFORMED;
                        }
                        playListType = M3U8_TYPE_MASTER;
                    }
                    if(StringHelper::startWith(line, EXT_X_I_FRAME_STREAM_INF)) {
                        if(playListType == M3U8_TYPE_MASTER) {
                            return ERROR_MALFORMED;
                        }
                        playListType = M3U8_TYPE_MASTER;
                    }
                    if(StringHelper::startWith(line, EXT_X_SESSION_DATA)) {
                        if(playListType == M3U8_TYPE_MASTER) {
                            return ERROR_MALFORMED;
                        }
                        playListType = M3U8_TYPE_MASTER;
                    }
                    if(StringHelper::startWith(line, EXT_X_SESSION_KEY)) {
                        if(playListType == M3U8_TYPE_MASTER) {
                            return ERROR_MALFORMED;
                        }
                        playListType = M3U8_TYPE_MASTER;
                    }

                    //parse media playlist
                    if(M3UMedia::isMediaPlayListTag(line)) {
                        if(playListType == M3U8_TYPE_MASTER) {
                            return ERROR_MALFORMED;
                        }
                        playListType = M3U8_TYPE_MEDIA;
                        if(m3uMedia == NULL) {
                            m3uMedia = new M3UMedia();
                        }
                        m3uMedia->parseMediaPlayList(line);
                        continue;
                    }

                    //parse media m3u8
                    if(MediaSegment::isMediaSegmentTag(line)) {
                        if(playListType == M3U8_TYPE_MASTER) {
                            return ERROR_MALFORMED;
                        }
                        playListType = M3U8_TYPE_MEDIA;
                        if(curMediaSegment == NULL) {
                            curMediaSegment = new MediaSegment();
                        }
                        curMediaSegment->parseMediaSegemnt(line);
                        continue;
                    }

                    if(!StringHelper::startWith(line, "#")) {
                        if(playListType == M3U8_TYPE_MEDIA && curMediaSegment != NULL) {
                            curMediaSegment->parseMediaSegemnt(line);
                            mMediaSegmentVector.push_back(curMediaSegment);
                            //curMediaSegment->dump();
                            if(m3uMedia == NULL) {
                                m3uMedia = new M3UMedia();
                            }
                            if(curMediaSegment != NULL) {
                                m3uMedia->pushMediaSegment(*curMediaSegment);
                            }
                            delete curMediaSegment;
                            curMediaSegment = NULL;
                        }
                        continue;
                    }
                }
            }

            if((playListType == M3U8_TYPE_MEDIA) && (m3uMedia != NULL)) {
                m3uMedia->setM3UVersion(m3u8Version);
                //(*m3uMedia).dump();
            }

            std::cout<<"size is "<<mMediaSegmentVector.size()<<std::endl;
            return -1;
        }
};
