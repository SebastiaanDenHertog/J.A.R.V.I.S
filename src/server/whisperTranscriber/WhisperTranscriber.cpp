/*
Create by: Sebastiaan den Hertog
Date: 2024-09-01

Description:
*/
#include "WhisperTranscriber.h"
#include <iostream>
#include <sstream>

WhisperTranscriber::WhisperTranscriber() : ctx_(nullptr) {}

WhisperTranscriber::~WhisperTranscriber()
{
    if (ctx_)
    {
        whisper_free(ctx_);
    }
}

bool WhisperTranscriber::setup(const Params &params)
{
    std::lock_guard<std::mutex> lock(whisper_mutex_);
    params_ = params;

    struct whisper_context_params cparams = whisper_context_default_params();
    cparams.use_gpu = params_.use_gpu;

    ctx_ = whisper_init_from_file_with_params(params_.model_path.c_str(), cparams);
    if (!ctx_)
    {
        std::cerr << "Failed to initialize whisper context" << std::endl;
        return false;
    }

    return true;
}

std::string WhisperTranscriber::transcribeLiveData(const std::vector<float> &pcmf32)
{
    std::lock_guard<std::mutex> lock(whisper_mutex_);
    return processTranscription(pcmf32);
}

std::string WhisperTranscriber::processTranscription(const std::vector<float> &pcmf32)
{
    whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    wparams.language = params_.language.c_str();
    wparams.n_threads = params_.n_threads;
    wparams.translate = params_.translate;

    if (whisper_full_parallel(ctx_, wparams, pcmf32.data(), pcmf32.size(), params_.n_processors) != 0)
    {
        std::cerr << "Failed to process live audio" << std::endl;
        return "";
    }

    std::stringstream result;
    const int n_segments = whisper_full_n_segments(ctx_);
    for (int i = 0; i < n_segments; ++i)
    {
        result << whisper_full_get_segment_text(ctx_, i) << "\n";
    }

    return result.str();
}
