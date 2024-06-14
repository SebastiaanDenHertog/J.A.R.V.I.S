#ifndef MODEL_RUNNER_H
#define MODEL_RUNNER_H

#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/model.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "Task.h"

class ModelRunner
{
public:
    ModelRunner(const std::unordered_map<std::string, std::string> &model_paths);
    bool IsLoaded(const std::string &model_name) const;
    bool RunInference(const std::string &model_name, const std::string &input_text, std::vector<float> &result);
    std::string predictTaskFromInput(const std::string &model_name, const std::string &input);

    void LoadClassNames(const std::unordered_map<std::string, std::string> &class_files);

private:
    bool LoadModel(const std::string &model_name, const std::string &model_path);
    std::vector<int> prepare_input(const std::string &input_text, const std::unordered_map<std::string, int> &tokenizer, int max_length);

    std::unordered_map<std::string, std::unique_ptr<tflite::FlatBufferModel>> models_;
    std::unordered_map<std::string, std::unique_ptr<tflite::Interpreter>> interpreters_;
    std::unordered_map<std::string, std::vector<std::string>> class_names_;
    std::unordered_map<std::string, std::unordered_map<std::string, int>> tokenizers_;
};

#endif // MODEL_RUNNER_H
