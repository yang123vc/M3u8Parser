#include <iostream>
#include <string.h>
#include <stdlib.h>

#include "HLSError.h"

#ifndef __STRINGHELPER_H__
#define __STRINGHELPER_H__

class StringHelper{
    public:
        static bool startWith(const std::string& str, const std::string& subStr) {
            if(strncmp(str.c_str(), subStr.c_str(), subStr.length()) == 0) {
                return true;
            } 
            return false;
        }

        static std::string getLine(const char* data, int *offsetCur, int size) {
            int offset = *offsetCur;
            int offsetLF = offset;
            //std::cout<<"offset is "<<offset<<std::endl;
            std::string line = "";
            if(offset < size) {
                while(offsetLF < size && data[offsetLF] != '\n') {
                    offsetLF++;
                }
                //std::cout<<"offsetLF is "<<offsetLF<<std::endl;
                if(offset < offsetLF && data[offsetLF - 1] == '\r') {
                    line.assign(&data[offset], offsetLF - offset - 1);
                } else {
                    line.assign(&data[offset], offsetLF - offset);
                }
            }
            *offsetCur = offsetLF + 1;
            //std::cout<<"getLine : "<<line<<std::endl;
            return line;
        }

        static std::map<std::string, std::string>* parseAttr(std::string line) {
            std::map<std::string, std::string> *attrMap = new std::map<std::string, std::string>();
            char attr[1024];
            memset(attr, 0, sizeof(attr));
            strncpy(attr, line.c_str(), sizeof(attr) - 1);
            char *p = strtok(attr, ":");
            while(p) {
                p = strtok(NULL, ",");
                if(p) {
                    printf("attr is %s\n", p);
                    p = strtok(NULL, "=");
                }
            }
            return attrMap;
        }

        static bool parseDouble(const char* s, double *x) {
            char *end = NULL;
            double val = strtod(s, &end);
            if(end == s || (*end != '\0' && *end != ',')) {
                printf("parse double error \n");
                return false;
            }
            *x = val;
            return true;
        }

        static bool parseInt32(const char* s, int *x) {
            char *end = NULL;
            int val = strtol(s, &end, 10);
            if(end == s || (*end != '\0' && *end != ',')) {
                printf("parse double error \n");
                return false;
            }
            *x = val;
            return true;
        }

};
#endif
