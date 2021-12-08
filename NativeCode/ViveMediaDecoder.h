//========= Copyright 2015-2019, HTC Corporation. All rights reserved. ===========

#pragma once

extern "C" {
	// Utils
	__declspec(dllexport) void nativeCleanAll();
	//	Decoder
	__declspec(dllexport) int nativeCreateDecoder(const char* filePath, int& id);
	__declspec(dllexport) int nativeCreateDecoderAsync(const char* filePath, int& id);
	__declspec(dllexport) int nativeGetDecoderState(int id);
	__declspec(dllexport) bool nativeStartDecoding(int id);
	__declspec(dllexport) void nativeDestroyDecoder(int id);
	__declspec(dllexport) bool nativeIsEOF(int id);
	__declspec(dllexport) void nativeGrabVideoFrame(int id, void** frameData, bool& frameReady);
	__declspec(dllexport) void nativeReleaseVideoFrame(int id);
	//	Video
	__declspec(dllexport) bool nativeIsVideoEnabled(int id);
	__declspec(dllexport) void nativeSetVideoEnable(int id, bool isEnable);
	__declspec(dllexport) void nativeGetVideoFormat(int id, int& width, int& height, float& totalTime);
	__declspec(dllexport) void nativeSetVideoTime(int id, float currentTime);
	__declspec(dllexport) bool nativeIsContentReady(int id);
	__declspec(dllexport) bool nativeIsVideoBufferFull(int id);
	__declspec(dllexport) bool nativeIsVideoBufferEmpty(int id);
	//	Audio
	__declspec(dllexport) bool nativeIsAudioEnabled(int id);
	__declspec(dllexport) void nativeSetAudioEnable(int id, bool isEnable);
	__declspec(dllexport) void nativeSetAudioAllChDataEnable(int id, bool isEnable);
	__declspec(dllexport) void nativeGetAudioFormat(int id, int& channel, int& frequency, float& totalTime);
	__declspec(dllexport) float nativeGetAudioData(int id, unsigned char** audioData, int& frameSize);
	__declspec(dllexport) void nativeFreeAudioData(int id);
	//	Seek
	__declspec(dllexport) void nativeSetSeekTime(int id, float sec);
	__declspec(dllexport) bool nativeIsSeekOver(int id);
	//  Utility
	__declspec(dllexport) int nativeGetMetaData(const char* filePath, char*** key, char*** value);
}