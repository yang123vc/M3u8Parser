/**
 * author: oujiangping@dvt.dvt.com
 * date: 2016/06/15
 * describe: for the master playlist
 */

#include "M3U8Base.h"

//目前暂时只实现ext-x-stream-inf的bandwidth节点
class M3UMaster : public M3UBase{
    private:
        list<M3UMedia> *m3uMediaList;
    public:
        list<M3uMedia>* getM3uMediaList();
        M3uMedia* getM3uMediaByBandWidth(int curBandWidth);
}
