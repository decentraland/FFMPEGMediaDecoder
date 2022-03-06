#include "VideoPlayer.h"
#include "Logger.h"
int main()
{
    logging("Hello world\n");

    VideoPlayerContext* vpContext = create("http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4");
    if (vpContext == NULL)
    {
        logging("error on create");
        return -1;
    }

    int i = 0;
    while(i <= 2000)
    {
        int res = process_frame(vpContext);
        if (res >= 0) {
            ++i;
        }
    }

    destroy(vpContext);
}