#include <iostream>
#include "ViveMediaDecoder.h"

int main(int argc, char** argv)
{
    nativeCleanAll();

    std::cout << "Init" << std::endl;
    int id = 0;
    int width = 0, height = 0;
    float totalTime = 0.0f;

    clock_t globalStartTime = clock();

    nativeCreateDecoderAsync("http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4", id);

    while(true) {
        int res = nativeGetDecoderState(id);
        std::cout << "Res: " << res << std::endl;
        if (res >= 1) break;
    }

    nativeGetVideoFormat(id, width, height, totalTime);

    nativeStartDecoding(id);
    int i = 0;
    while(i != 45) {
        void* frameData = nullptr;
        bool newFrame = false;
        nativeGrabVideoFrame(id, &frameData, newFrame);

        if (newFrame) {
            ++i;
            std::cout << "Frame: " << i << std::endl;
            nativeReleaseVideoFrame(id);
        }
    }

    nativeDestroyDecoder(id);
    std::cout << "Done!" << std::endl;
    return 0;
}