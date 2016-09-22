#ifndef __M3UMEDIA__H__
#define __M3UMEDIA_H__

#include "M3U8Base.h"

class M3UMedia : public M3U8Base {
    private:
        int targetDuration;
        int sequence;
        int bandWidth;
        bool hasEndTag;
        std::list<MediaSegment> mediaSegments;
    public:
        M3UMedia():
            sequence(0),
            bandWidth(0),
            hasEndTag(false),
            targetDuration(0) {
            }

        static bool parseTargetDuration(const std::string &line, int *x) {
            int colonPos = line.find(":");
            if(colonPos < 0) {
                return ERROR_MALFORMED;
            }
            bool ret = StringHelper::parseInt32(line.c_str() + colonPos + 1, x);
            return ret;
        }

        static bool parseMediaSequence(const std::string &line, int *x) {
            int colonPos = line.find(":");
            if(colonPos < 0) {
                return ERROR_MALFORMED;
            }
            bool ret = StringHelper::parseInt32(line.c_str() + colonPos + 1, x);
            return ret;
        }

        // #EXT-X-TARGETDURATION
        int getTargetDuration() {
            return targetDuration;
        }

        int setTargetDuration(int v) {
            targetDuration = v;
        }

        //#EXT-X-MEDIA-SEQUENCE
        int getMediaSequence() {
            return sequence;
        }

        int setMediaSequence(int v) {
            sequence = v;
        }

        // #EXT-X-ENDLIST
        bool hasEndList() {
            return hasEndTag;
        }

        bool parseMediaPlayList(const std::string line) {
            if(StringHelper::startWith(line, EXT_X_TARGETDURATION)) {
                if(!parseTargetDuration(line, &targetDuration)) {
                    targetDuration = 0;
                }
                std::cout<<"targetDuration is "<<targetDuration<<std::endl;
                return true;
            }

            if(StringHelper::startWith(line, EXT_X_MEDIA_SEQUENCE)) {
                if(!parseMediaSequence(line, &sequence)) {
                    sequence = 0;
                }
                std::cout<<"sequence is "<<sequence<<std::endl;
                return true;
            }

            if(line == EXT_X_ENDLIST) {
                hasEndTag = true;
                std::cout<<"find endlist ..........."<<std::endl;
                return true;
            }

            return false;
        }

        static bool isMediaPlayListTag(std::string line) {
            static std::vector<std::string> tagsVector = std::vector<std::string>();
            if(tagsVector.size() <= 0) {
                tagsVector.push_back(EXT_X_TARGETDURATION);
                tagsVector.push_back(EXT_X_MEDIA_SEQUENCE);
                tagsVector.push_back(EXT_X_DISCONTINUITY_SEQUENCE);
                tagsVector.push_back(EXT_X_ENDLIST);
                tagsVector.push_back(EXT_X_PLAYLIST_TYPE);
                tagsVector.push_back(EXT_X_I_FRAMES_ONLY);
            }
            for(int i = 0; i < tagsVector.size(); i++) {
                if(StringHelper::startWith(line, tagsVector[i])) {
                    return true;
                }
            }
            return false;
        }

        bool pushMediaSegment(MediaSegment segment) {
            mediaSegments.push_back(segment);
            return true;
        }

        void dump() {
            std::cout<<"--------------- dump M3UMedia ----------------"<<std::endl;
            std::cout<<"version: "<<version<<std::endl;
            std::cout<<"targetDuration: "<<targetDuration<<std::endl;
            std::cout<<"mediaSequence: "<<sequence<<std::endl;
            std::cout<<"bandWidth: "<<bandWidth<<std::endl;
            std::cout<<"hasEndTag: "<<hasEndTag<<std::endl;
            for(std::list<MediaSegment>::iterator it = mediaSegments.begin(); it != mediaSegments.end(); ++it) {
                (*it).dump();
            }
            std::cout<<"--------------- end dump M3UMedia ----------------"<<std::endl;
        }
};

#endif
