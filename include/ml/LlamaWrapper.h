/*
Create by: Sebastiaan den Hertog
Date: 2024-09-03

Description:
*/

#ifndef LLAMAWRAPPER_H
#define LLAMAWRAPPER_H

#include <arg.h>
#include <common.h>
#include <console.h>
#include <log.h>
#include <sampling.h>
#include <llama.h>

#include <vector>
#include <string>
#include <sstream>
#include <csignal>
#include <cstdio>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

class LlamaWrapper
{
public:
    LlamaWrapper(int argc, char **argv);
    ~LlamaWrapper();
    void addSentence(const std::string &sentence, std::function<void(const std::string &)> callback);

    void run();
    void stop();

private:
    llama_context *ctx;
    llama_model *model;
    gpt_sampler *sampler;
    static gpt_params *g_params;
    gpt_params params;
    std::vector<llama_token> input_tokens;
    std::vector<llama_token> output_tokens;
    std::ostringstream output_ss;
    bool is_interacting;
    bool need_insert_eot;

    struct SentenceJob
    {
        std::string sentence;
        std::function<void(const std::string &)> callback;
    };

    std::queue<SentenceJob> sentenceQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    bool running;

    void processQueue();

    std::vector<llama_token> tokenize_input(const std::string &prompt, bool add_bos = true);
    bool file_exists(const std::string &path);
    bool file_is_empty(const std::string &path);
    void write_logfile(const std::vector<llama_token> &input_tokens, const std::string &output);
    std::string chat_add_and_format(std::vector<llama_chat_msg> &chat_msgs, const std::string &role, const std::string &content);
    void handle_interrupt();
    void interactive_mode();
    void process_generation(std::vector<llama_token> &embd_inp, std::vector<llama_chat_msg> &chat_msgs);
};

#endif // LLAMA_WRAPPER_H
