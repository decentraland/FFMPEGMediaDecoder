#include "decoder.h"
#include "logger.h"

int runDecoder()
{
    logging("Running Decoder\n");

    DecoderContext* vpContext = create("http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4");
    if (vpContext == NULL)
    {
        logging("error on create");
        return -1;
    }

    //int i = 0;
    while(0 == 0)
    {
        ProcessOutput processOutput;
        int res = process_frame(vpContext, &processOutput);

        if (processOutput.videoFrame) {
            av_frame_free(&processOutput.videoFrame);
        }

        if (processOutput.audioFrame) {
            av_frame_free(&processOutput.audioFrame);
        }

        /*if (res >= 0) {
            ++i;
        }*/
    }

    destroy(vpContext);
    return 0;
}

int main()
{
    logging("Hello world\n");

    return runDecoder();
}