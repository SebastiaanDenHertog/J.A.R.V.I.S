/*
Create by: Sebastiaan den Hertog
Date: 2024-09-03

Description:
*/

#include "LlamaWrapper.h"
#include <iostream>
#include <fstream>

gpt_params *LlamaWrapper::g_params = nullptr;

LlamaWrapper::LlamaWrapper(int argc, char **argv) : is_interacting(false), need_insert_eot(false)
{
    
    g_params = &params;
    if (!gpt_params_parse(argc, argv, params, LLAMA_EXAMPLE_MAIN, [](int argc, char **argv)
                          {}))
    {
        exit(1);
    }

    gpt_init();
    console::init(params.simple_io, params.use_color);
    atexit([]()
           { console::cleanup(); });

    if (params.n_ctx != 0 && params.n_ctx < 8)
    {
        params.n_ctx = 8;
    }

    llama_backend_init();
    llama_numa_init(params.numa);

    llama_init_result llama_init = llama_init_from_gpt_params(params);
    model = llama_init.model;
    ctx = llama_init.context;

    if (!model)
    {
        LOG_ERR("%s: error: unable to load model\n", __func__);
        exit(1);
    }

    sampler = gpt_sampler_init(model, params.sparams);
    if (!sampler)
    {
        LOG_ERR("%s: failed to initialize sampling subsystem\n", __func__);
        exit(1);
    }
}

LlamaWrapper::~LlamaWrapper()
{
    gpt_sampler_free(sampler);
    llama_free(ctx);
    llama_free_model(model);
    llama_backend_free();
}

void LlamaWrapper::run()
{
    std::vector<llama_chat_msg> chat_msgs;
    std::vector<llama_token> embd_inp = tokenize_input(params.prompt);

    if (params.interactive)
    {
        interactive_mode();
    }
    else
    {
        process_generation(embd_inp, chat_msgs);
    }
}

std::vector<llama_token> LlamaWrapper::tokenize_input(const std::string &prompt, bool add_bos)
{
    std::vector<llama_token> tokens = ::llama_tokenize(ctx, prompt, add_bos, true);
    return tokens;
}

bool LlamaWrapper::file_exists(const std::string &path)
{
    std::ifstream f(path.c_str());
    return f.good();
}

bool LlamaWrapper::file_is_empty(const std::string &path)
{
    std::ifstream f;
    f.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    f.open(path.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
    return f.tellg() == 0;
}

void LlamaWrapper::write_logfile(const std::vector<llama_token> &input_tokens, const std::string &output)
{
    if (params.logdir.empty())
        return;

    const std::string timestamp = string_get_sortable_timestamp();
    const bool success = fs_create_directory_with_parents(params.logdir);
    if (!success)
    {
        LOG_ERR("%s: failed to create logdir %s, cannot write logfile\n", __func__, params.logdir.c_str());
        return;
    }

    const std::string logfile_path = params.logdir + timestamp + ".yml";
    FILE *logfile = fopen(logfile_path.c_str(), "w");
    if (!logfile)
    {
        LOG_ERR("%s: failed to open logfile %s\n", __func__, logfile_path.c_str());
        return;
    }

    fprintf(logfile, "binary: main\n");
    char model_desc[128];
    llama_model_desc(model, model_desc, sizeof(model_desc));
    yaml_dump_non_result_info(logfile, params, ctx, timestamp, input_tokens, model_desc);

    fprintf(logfile, "\n######################\n");
    fprintf(logfile, "# Generation Results #\n");
    fprintf(logfile, "######################\n\n");

    yaml_dump_string_multiline(logfile, "output", output.c_str());
    yaml_dump_vector_int(logfile, "output_tokens", output_tokens);
    llama_perf_dump_yaml(logfile, ctx);
    fclose(logfile);
}

std::string LlamaWrapper::chat_add_and_format(std::vector<llama_chat_msg> &chat_msgs, const std::string &role, const std::string &content)
{
    llama_chat_msg new_msg{role, content};
    auto formatted = llama_chat_format_single(model, params.chat_template, chat_msgs, new_msg, role == "user");
    chat_msgs.push_back({role, content});
    return formatted;
}

void LlamaWrapper::handle_interrupt()
{
    console::cleanup();
    gpt_perf_print(ctx, sampler);
    write_logfile(input_tokens, output_ss.str());
    LOG("Interrupted by user\n");
    gpt_log_pause(gpt_log_main());
    exit(130);
}

void LlamaWrapper::interactive_mode()
{
    LOG_INF("Interactive mode enabled.\n");
    // Add specific interactive mode handling logic
}

void LlamaWrapper::process_generation(std::vector<llama_token> &embd_inp, std::vector<llama_chat_msg> &chat_msgs)
{
    std::ostringstream assistant_ss;

    if (params.interactive_first || !params.prompt.empty())
    {
        embd_inp = tokenize_input(params.prompt);
    }

    int n_past = 0;
    int n_remain = params.n_predict;
    int n_consumed = 0;

    while (n_remain != 0)
    {
        if (!embd_inp.empty())
        {
            for (int i = 0; i < (int)embd_inp.size(); i += params.n_batch)
            {
                int n_eval = (int)embd_inp.size() - i;
                if (n_eval > params.n_batch)
                    n_eval = params.n_batch;

                llama_decode(ctx, llama_batch_get_one(&embd_inp[i], n_eval, n_past, 0));
                n_past += n_eval;
            }
            embd_inp.clear();
        }

        const llama_token id = gpt_sampler_sample(sampler, ctx, -1);
        embd_inp.push_back(id);
        output_ss << llama_token_to_piece(ctx, id);
        --n_remain;
    }
}