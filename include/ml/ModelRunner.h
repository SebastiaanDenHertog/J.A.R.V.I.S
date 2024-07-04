#ifndef MODEL_RUNNER_H
#define MODEL_RUNNER_H

#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/model.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/op_resolver.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "Task.h"

class ModelRunner
{
public:
    ModelRunner(const std::unordered_map<std::string, std::string> &model_paths);
    bool IsLoaded(const std::string &model_name);
    void LoadTokenizer(const std::string &tokenizer_path);
    void LoadLabels(const std::string &ner_labels_path, const std::string &command_type_labels_path);
    bool RunInference(const std::string &input_text, std::vector<std::vector<float>> &ner_result, std::vector<float> &command_type_result);
    std::pair<std::string, std::vector<std::string>> predictTaskFromInput(const std::string &input);

private:
    std::vector<int> TokenizeInput(const std::string &input_text);

    std::unordered_map<std::string, std::unique_ptr<tflite::FlatBufferModel>> models_;
    std::unordered_map<std::string, std::unique_ptr<tflite::Interpreter>> interpreters_;
    std::unordered_map<int, std::string> ner_labels_;
    std::unordered_map<int, std::string> command_type_labels_;
    std::unordered_map<int, std::string> tokenizer_index_word_;
    std::unordered_map<std::string, int> tokenizer_word_index_;
    int max_length_;
};

#endif // MODEL_RUNNER_H