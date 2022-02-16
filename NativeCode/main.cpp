#include <iostream>
#include "ViveMediaDecoder.h"

enum class DecoderState {
    INITIALIZING,
    START,
    BUFFERING,
    SEEK_FRAME,
    PAUSE,
    EndOfFile
};

int main(int argc, char** argv)
{
    nativeCleanAll();

    std::cout << "Init3" << std::endl;
    int decoderID = 0;
    int width = 0, height = 0;
    float videoTotalTime = 0.0f;

    nativeCreateDecoderAsync("https://bafybeia7pyfmyjcnjrodau2yxm3h3bs6suv3xfzwvhohktnw6v2cevayaq.ipfs.dweb.link/mp4/0/0_Action-01-Look_R-L.mp4", decoderID);
    //nativeCreateDecoderAsync("http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4", decoderID);

    while(true) {
        int res = nativeGetDecoderState(decoderID);
        if (res >= 1) break;
    }

    nativeGetVideoFormat(decoderID, width, height, videoTotalTime);

    nativeStartDecoding(decoderID);

    clock_t globalStartTime = clock();
    float hangTime = 0.0f;
    int i = 0;
    float time = 0.0f;

    bool isVideoEnabled = true;
    DecoderState decoderState = DecoderState::BUFFERING;
    DecoderState lastState = DecoderState::BUFFERING;
    bool seekPreview = false;

    DecoderState lastShowedState = DecoderState::START;

    while(true) {
        if (decoderState != lastShowedState) {
            std::cout << "DecoderState: " << (int) decoderState << std::endl;
            lastShowedState = decoderState;
        }

        if (decoderState >= DecoderState::START && decoderState != DecoderState::SEEK_FRAME && nativeIsAudioEnabled(decoderID)) {
            unsigned char *audioFrameData = nullptr;
            int audioFrameSize = 0;
            double nativeAudioTime = nativeGetAudioData(decoderID, &audioFrameData, audioFrameSize);
            if (nativeAudioTime != -1.0)
                nativeFreeAudioData(decoderID);
        }

        switch (decoderState) {
            case DecoderState::START:
                if (isVideoEnabled) {
                    void* frameData = nullptr;
                    bool newFrame = false;
                    nativeGrabVideoFrame(decoderID, &frameData, newFrame);

                    if (newFrame) {
                        ++i;
                        std::cout << "Frame: " << i << std::endl;
                        nativeReleaseVideoFrame(decoderID);
                    }

                    //	Update video frame by dspTime.
                    double setTime = (clock() - globalStartTime);

                    //	Normal update frame.
                    if (setTime < videoTotalTime || videoTotalTime == -1.0f) {
                        if (seekPreview && nativeIsContentReady(decoderID)) {
                            //setPause();
                            seekPreview = false;
                        } else {
                            nativeSetVideoTime(decoderID, (float) setTime);
                        }
                    } else {
                        if (!nativeIsVideoBufferEmpty(decoderID)) {
                            nativeSetVideoTime(decoderID, (float)setTime);
                        }
                    }
                }

                if (nativeIsVideoBufferEmpty(decoderID) && !nativeIsEOF(decoderID)) {
                    decoderState = DecoderState::BUFFERING;
                    hangTime = clock() - globalStartTime;
                }

                break;

            case DecoderState::SEEK_FRAME:
                if (nativeIsSeekOver(decoderID)) {
                    globalStartTime = clock() - hangTime;
                    decoderState = DecoderState::START;
                    if (lastState == DecoderState::PAUSE) {
                        seekPreview = true;
                        //mute();
                    }
                }
                break;

            case DecoderState::BUFFERING:
                if (nativeIsVideoBufferFull(decoderID) || nativeIsEOF(decoderID)) {
                    decoderState = DecoderState::START;
                    globalStartTime = clock() - hangTime;
                }
                break;

            case DecoderState::PAUSE:
            case DecoderState::EndOfFile:
            default:
                break;
        }

        if (!isVideoEnabled) {
            break;
        }
    }

    nativeDestroyDecoder(decoderID);
    std::cout << "Done!" << std::endl;
    return 0;
}