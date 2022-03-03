//========= Copyright 2015-2019, HTC Corporation. All rights reserved. ===========

#pragma once

#if defined(_WIN32) && defined(MYLIB_DLL)
//   Compiling a Windows DLL
#    define export __declspec(dllexport)
// Windows or Linux static library, or Linux so
#else
#    define export
#endif

extern "C" {
    // Utils
    export void nativeCleanAll();
    export void nativeCleanDestroyedDecoders();
	//	Decoder
	export int nativeCreateDecoder(const char* filePath, int& id);
	export int nativeCreateDecoderAsync(const char* filePath, int& id);
	export int nativeGetDecoderState(int id);
	export bool nativeStartDecoding(int id);
    export void nativeScheduleDestroyDecoder(int id);
	export void nativeDestroyDecoder(int id);
	export bool nativeIsEOF(int id);
    export void nativeGrabVideoFrame(int id, void** frameData, bool& frameReady);
    export void nativeReleaseVideoFrame(int id);
	//	Video
	export bool nativeIsVideoEnabled(int id);
	export void nativeSetVideoEnable(int id, bool isEnable);
	export void nativeGetVideoFormat(int id, int& width, int& height, float& totalTime);
	export void nativeSetVideoTime(int id, float currentTime);
	export bool nativeIsContentReady(int id);
	export bool nativeIsVideoBufferFull(int id);
	export bool nativeIsVideoBufferEmpty(int id);
	//	Audio
	export bool nativeIsAudioEnabled(int id);
	export void nativeSetAudioEnable(int id, bool isEnable);
	export void nativeSetAudioAllChDataEnable(int id, bool isEnable);
	export void nativeGetAudioFormat(int id, int& channel, int& frequency, float& totalTime);
	export float nativeGetAudioData(int id, unsigned char** audioData, int& frameSize);
	export void nativeFreeAudioData(int id);
	//	Seek
	export void nativeSetSeekTime(int id, float sec);
	export bool nativeIsSeekOver(int id);
}