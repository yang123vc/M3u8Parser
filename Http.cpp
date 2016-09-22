#include <iostream>
#include <stdio.h>
#include <stdexcept>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/http.h>
#include <event2/dns.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdlib.h>

#include "M3U8Parser.h"
#include "MediaSegment.h"



class HttpExeption : public std::logic_error {
    public:
        HttpExeption(const std::string & msg = "Unknow HttpExeption") : std::logic_error(msg) {
        }

        void printError() {
            std::cout<<what()<<std::endl;
        }
};

class Http {
    protected:
        struct evhttp_uri *httpUri;
        struct event_base *base;
        struct bufferevent *bev;
        struct evhttp_connection *evcon;
        struct evhttp_request *req;
        struct evdns_base *dnsbase;
        std::string url;
        int port;
        std::string path;
        std::string host;
        std::string scheme;
        std::string uperUrl;
        int maxRetries;

        void (*myRequestCallBack)(struct evhttp_request *, void *);
    public:
        static const int max_url_len = 2048;
    public:
        Http(const char *url) throw(HttpExeption) :
            httpUri(NULL),
            base(NULL),
            bev(NULL),
            evcon(NULL),
            req(NULL),
            dnsbase(NULL),
            maxRetries(0) {
            if(!setUrl(url)) {
                throw HttpExeption("Create HttpExeption error");
            }
            //initHttpConn();
        }

        void setRequestCallBack(void (*myRequestCallBack1)(struct evhttp_request *, void *)) {
            myRequestCallBack = myRequestCallBack1;
        }

        std::string getUperUrl() {
            return uperUrl;
        }

        bool setUrl(const char *url) {
            if(url == NULL) {
                return false;
            }
            int len = strlen(url);
            if(len > 0 && len < max_url_len) {
                this->url = url;
            } else {
                return false;
            }

            httpUri = evhttp_uri_parse(url);
            port = evhttp_uri_get_port(httpUri);
            host = evhttp_uri_get_host(httpUri);
            path = evhttp_uri_get_path(httpUri);
            scheme = evhttp_uri_get_scheme(httpUri);

            if(port == -1) {
                port = 80;
            }
            if(port < 0) {
                return false;
            }
            std::size_t lastSeparator = this->url.rfind("/");
            if(lastSeparator > (scheme.length() + strlen("://"))) {
                uperUrl = this->url.substr(0, lastSeparator + 1);
            } else {
                uperUrl = this->url + "/";
            }
            return true;
        }

        void setRetries(int max) {
            maxRetries = max;
        }


        bool addHeader(const char* key, const char* value) {
            if(req == NULL) {
                return false;
            }
            struct evkeyvalq *reqHeader = evhttp_request_get_output_headers(req);
            evhttp_add_header(reqHeader, key, value);
            return true;
        }


        static void requestCallBack(struct evhttp_request *req, void *ctx) {
            std::cout<<"test requestCallBack\n"<<std::endl;
        }

        bool initHttpConn() {
            if(myRequestCallBack == NULL) {
                return false;
            }
            base = event_base_new();
            bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
            evcon = evhttp_connection_base_bufferevent_new(base, dnsbase, bev, (const char*)host.c_str(), (unsigned short)port);
            evhttp_connection_set_retries(evcon, maxRetries);
            req = evhttp_request_new(myRequestCallBack, bev);
            if(!base || !bev || !evcon) {
                return false;
            }
            return true;
        }

        bool request() {
            evhttp_make_request(evcon, req, EVHTTP_REQ_GET, url.c_str());
            event_base_dispatch(base);
            return true;
        }

        std::string getHost() {
            return host;
        }

        bool initHost() {
            return addHeader("Host", host.c_str());
        }

        bool setConnectClose() {
            return addHeader("Connection", "close");
        }

        void dump() {
            std::cout<<"url:"<<url<<"scheme: "<<scheme<<" host:"<<host<<" port:"<<port<<" path:"<<path<<std::endl;
            std::cout<<"uperUrl:"<<uperUrl<<std::endl;
        }

        static int getContentLength(struct evhttp_request *req) {
            struct evkeyvalq *responseHeader = evhttp_request_get_input_headers(req);
            const char* contentLen = evhttp_find_header(responseHeader, "Content-Length");
            std::cout<<"contentLen: "<<contentLen<<std::endl;
            return atoi(contentLen);
        }
};

M3U8Parser *parser;
std::string currentTsPath;
std::string currentTsWritePath;

static void TsRequestCallBack(struct evhttp_request *req, void *ctx) {
    FILE *tsFile = fopen(currentTsWritePath.c_str(), "w+");
    int nread = 0;
    unsigned char* buf[1024];
    std::cout<<"begin write "<<currentTsPath<<std::endl;
    if(req == NULL) {
        int errcode = EVUTIL_SOCKET_ERROR();
        std::cout<<"socket error "<<evutil_socket_error_to_string(errcode)<<" code "<<errcode<<std::endl;
        return;
    }
    while((nread = evbuffer_remove(evhttp_request_get_input_buffer(req),
                    buf, sizeof(buf) - 1)) > 0) {
        fwrite(buf, sizeof(unsigned char), nread, tsFile);
    }
    std::cout<<"end write "<<currentTsPath<<std::endl;
    fclose(tsFile);
}


static void M3u8RequestCallBack(struct evhttp_request *req, void *ctx) {
    int contentLen = Http::getContentLength(req);
    int nread = 0;
    char* buf = new char[contentLen];
    char* httpBuf = new char[1500];
    int currentPos = 0;
    std::cout<<"start parse m3u8"<<std::endl;
    if(req == NULL) {
        int errcode = EVUTIL_SOCKET_ERROR();
        std::cout<<"socket error "<<evutil_socket_error_to_string(errcode)<<" code "<<errcode<<std::endl;
        return;
    }
    while((nread = evbuffer_remove(evhttp_request_get_input_buffer(req),
                    httpBuf, sizeof(httpBuf) - 1)) > 0) {
        memcpy(buf+currentPos, httpBuf, nread);
        currentPos += nread;
    }
    //printf("%s\n", buf);
    parser = new M3U8Parser();
    parser->parser(buf, currentPos);
    std::cout<<"parse m3u8 end"<<std::endl;
}

class TsDownloadClient : public Http {
    private:
    public:
        TsDownloadClient(const char *url) :
            Http(url) {
                setRequestCallBack(TsRequestCallBack);
                initHttpConn();
            }

        ~TsDownloadClient() {
        }
};

class M3u8DownloadClient : public Http {
    private:
    public:
        M3u8DownloadClient(const char *url) :
            Http(url) {
                setRequestCallBack(M3u8RequestCallBack);
                initHttpConn();
            }

        ~M3u8DownloadClient() {
        }
};




int main(int argc, char **argv) {
    M3u8DownloadClient* http;
    try {
        http = new M3u8DownloadClient(argv[1]);
        http->initHost();
        http->setConnectClose();
    } catch(HttpExeption e) {
        std::cout<<e.what()<<std::endl;
        return -1;
    }
    http->request();
    M3UMedia *m3uMedia = parser->getM3uMedia();
    std::list<MediaSegment> mediaSegments = m3uMedia->getSegmentList();
    for(std::list<MediaSegment>::iterator it = mediaSegments.begin(); it != mediaSegments.end(); ++it) {
        currentTsPath = http->getUperUrl() + (*it).getUri();
        currentTsWritePath = "ts/" + (*it).getUri();
        TsDownloadClient tsClient(currentTsPath.c_str());
        try {
            TsDownloadClient tsClient = TsDownloadClient(currentTsPath.c_str());
            tsClient.initHost();
            tsClient.setConnectClose();
            tsClient.request();
        } catch(HttpExeption e) {
            std::cout<<e.what()<<std::endl;
            return -1;
        }
    }
    return 0;
}
