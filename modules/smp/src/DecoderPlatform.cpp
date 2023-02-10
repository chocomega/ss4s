#include "AVStreamPlayer.h"
#include "ss4s/modapi.h"

#include <cassert>


#include <memory>
#include <cstring>

#define decoder_init PLUGIN_SYMBOL_NAME(decoder_init)
#define decoder_check PLUGIN_SYMBOL_NAME(decoder_check)
#define decoder_finalize PLUGIN_SYMBOL_NAME(decoder_finalize)
#define decoder_callbacks PLUGIN_SYMBOL_NAME(decoder_callbacks)
#define audio_callbacks PLUGIN_SYMBOL_NAME(audio_callbacks)
#define presenter_callbacks PLUGIN_SYMBOL_NAME(presenter_callbacks)

using SMP_DECODER_NS::AudioConfig;
using SMP_DECODER_NS::AVStreamPlayer;
using SMP_DECODER_NS::VideoConfig;

static std::unique_ptr<AVStreamPlayer> streamPlayer;

static AudioConfig audioConfig;
static VideoConfig videoConfig;

extern "C" DECODER_RENDERER_CALLBACKS decoder_callbacks;
extern "C" AUDIO_RENDERER_CALLBACKS audio_callbacks;

logvprintf_fn MODULE_LOGVPRINTF = nullptr;

extern "C" bool decoder_init(int argc, char *argv[], const HOST_CONTEXT *hctx) {
    MODULE_LOGVPRINTF = hctx->logvprintf;
    return true;
}

extern "C" bool decoder_check(PDECODER_INFO dinfo) {
    dinfo->valid = true;
    dinfo->accelerated = true;
#if DEBUG
    // dinfo->audio = true;
    dinfo->audioConfig = AUDIO_CONFIGURATION_51_SURROUND;
#endif
    dinfo->hevc = true;
    dinfo->hdr = DECODER_HDR_SUPPORTED;
    dinfo->colorSpace = COLORSPACE_REC_709;
    dinfo->colorRange = COLOR_RANGE_FULL;
    dinfo->maxBitrate = 65000;
    dinfo->suggestedBitrate = 35000;
    return true;
}

extern "C" void decoder_finalize() {
}

static int _initPlayerWhenReady() {
    if (!streamPlayer)
        streamPlayer.reset(new AVStreamPlayer());
    // Don't set up before video comes in
    if (!videoConfig.format)
        return 0;
    streamPlayer->videoConfig = videoConfig;
    streamPlayer->audioConfig = audioConfig;
    if (!streamPlayer->load()) {
        videoConfig.format = 0;
        audioConfig.type = 0;
        return -1;
    }
    return 0;
}

static void _destroyPlayer() {
    streamPlayer.reset(nullptr);
}

static int videoSetup(int videoFormat, int width, int height, int redrawRate, void *context, int drFlags) {
    assert(!videoConfig.format);
    videoConfig.format = videoFormat;
    videoConfig.width = width;
    videoConfig.height = height;
    videoConfig.fps = redrawRate;
    return _initPlayerWhenReady();
}

static int _audioSetup(int audioConfiguration, const POPUS_MULTISTREAM_CONFIGURATION opusConfig, void *context,
                       int arFlags) {
    assert(!audioConfig.type);
    audioConfig.type = audioConfiguration;
    memcpy(&audioConfig.opusConfig, opusConfig, sizeof(OPUS_MULTISTREAM_CONFIGURATION));
    return _initPlayerWhenReady();
}

static int videoSubmit(PDECODE_UNIT decodeUnit) {
    if (!streamPlayer)
        return DR_OK;
    return streamPlayer->submitVideo(decodeUnit);
}

static void videoSetHdr(bool hdr) {
    if (!streamPlayer)
        return;
    streamPlayer->setHdr(hdr);
}

static void audioSubmit(char *sampleData, int sampleLength) {
    if (!streamPlayer)
        return;
    streamPlayer->submitAudio(sampleData, sampleLength);
}

static void avStop() {
    if (!streamPlayer)
        return;
    streamPlayer->sendEOS();
}

static void videoCleanup() {
    videoConfig.format = 0;
    _destroyPlayer();
}

static void audioCleanup() {
    audioConfig.type = 0;
    _destroyPlayer();
}

MODULE_API DECODER_RENDERER_CALLBACKS decoder_callbacks = {
        .setup = videoSetup,
        .start = nullptr,
        .stop = avStop,
        .cleanup = videoCleanup,
        .submitDecodeUnit = videoSubmit,
        .capabilities = CAPABILITY_SLICES_PER_FRAME(4) | CAPABILITY_DIRECT_SUBMIT,
};

MODULE_API AUDIO_RENDERER_CALLBACKS audio_callbacks = {
        .init = _audioSetup,
        .start = nullptr,
        .stop = avStop,
        .cleanup = audioCleanup,
        .decodeAndPlaySample = audioSubmit,
        .capabilities = CAPABILITY_DIRECT_SUBMIT,
};

MODULE_API VIDEO_PRESENTER_CALLBACKS presenter_callbacks = {
        .enterFullScreen = nullptr,
        .enterOverlay = nullptr,
        .setHdr = videoSetHdr,
};