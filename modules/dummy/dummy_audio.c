#include "dummy_common.h"

static bool GetAudioCapabilities(SS4S_AudioCapabilities *capabilities, SS4S_AudioCodec wantedCodecs) {
    if (!wantedCodecs) {
        wantedCodecs = SS4S_AUDIO_PCM_S16LE | SS4S_AUDIO_AAC | SS4S_AUDIO_AC3 | SS4S_AUDIO_EAC3;
    }
    capabilities->codecs = wantedCodecs;
    capabilities->maxChannels = 6;
    return true;
}

static SS4S_AudioOpenResult OpenAudio(const SS4S_AudioInfo *info, SS4S_AudioInstance **instance,
                                      SS4S_PlayerContext *context) {
    SS4S_Dummy_Log(SS4S_LogLevelInfo, "Dummy", "%s(codec=%s, numOfChannels=%d, sampleRate=%d, samplesPerFrame=%d)",
                   __FUNCTION__, SS4S_AudioCodecName(info->codec), info->numOfChannels, info->sampleRate,
                   info->samplesPerFrame);
    if (SS4S_Dummy_ReloadMedia(context) != 0) {
        return SS4S_AUDIO_OPEN_ERROR;
    }
    *instance = (SS4S_AudioInstance *) context;
    return SS4S_AUDIO_OPEN_OK;
}

static SS4S_AudioFeedResult FeedAudio(SS4S_AudioInstance *instance, const unsigned char *data, size_t size) {
    SS4S_PlayerContext *context = (void *) instance;
    if (!context->mediaLoaded) {
        return SS4S_AUDIO_FEED_NOT_READY;
    }
    return SS4S_AUDIO_FEED_OK;
}

static void CloseAudio(SS4S_AudioInstance *instance) {
    SS4S_Dummy_Log(SS4S_LogLevelInfo, "Dummy", "%s()", __FUNCTION__);
    SS4S_PlayerContext *context = (void *) instance;
    SS4S_Dummy_ReloadMedia(context);
}

const SS4S_AudioDriver SS4S_Dummy_AudioDriver = {
        .Base = {
                .PostInit = SS4S_Dummy_Driver_PostInit,
                .Quit = SS4S_Dummy_Driver_Quit,
        },
        .GetCapabilities = GetAudioCapabilities,
        .Open = OpenAudio,
        .Feed = FeedAudio,
        .Close = CloseAudio,
};