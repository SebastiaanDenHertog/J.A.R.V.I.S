/*
Create by: Sebastiaan den Hertog
Date: 2024-09-01

Description: This class is used to convert speech to text. It uses kaldi to do this. This function is only used on heavier devices to excuse the best quality.
*/

#ifndef WHISPERTRANSCRIBER_H
#define WHISPERTRANSCRIBER_H

#include "whisper.h"
#include <string>
#include <vector>
#include <mutex>

class WhisperTranscriber
{
public:
    struct Params
    {
        int n_threads = 4;
        int n_processors = 1;
        int offset_t_ms = 0;
        int duration_ms = 0;
        int max_context = -1;
        int max_len = 0;
        float word_thold = 0.01f;
        bool translate = false;
        bool diarize = false;
        bool use_gpu = true;
        std::string language = "en";
        std::string model_path = "models/ggml-base.en.bin";
    };

    WhisperTranscriber();
    ~WhisperTranscriber();

    // Sets up the Whisper transcriber with the given parameters
    bool setup(const Params &params);

    // Transcribes live audio data (PCM) to text
    std::string transcribeLiveData(const std::vector<float> &pcmf32);

private:
    Params params_;
    struct whisper_context *ctx_;
    std::mutex whisper_mutex_;

    // Internal function for processing transcription
    std::string processTranscription(const std::vector<float> &pcmf32);
};

#endif // WHISPERTRANSCRIBER_H
