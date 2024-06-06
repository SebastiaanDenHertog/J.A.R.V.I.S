#ifndef MODEL_RUNNER_H
#define MODEL_RUNNER_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <tensorflow/lite/model.h>
#include <tensorflow/lite/interpreter.h>
#include "Task.h"

// Forward declare Tokenizer class
class Tokenizer;

class ModelRunner
{
private:
    std::unordered_map<std::string, std::unique_ptr<tflite::FlatBufferModel>> models_;
    std::unordered_map<std::string, std::unique_ptr<tflite::Interpreter>> interpreters_;
    std::string user_context;

    bool LoadModel(const std::string &model_path, const std::string &model_name);
    bool CreateInterpreter(const std::string &model_name);

public:
    ModelRunner(const std::unordered_map<std::string, std::string> &model_paths);
    bool IsLoaded(const std::string &model_name) const;
    bool RunNLPModel(const std::string &input, std::vector<float> &result);
    bool RunLLMModel(const std::string &input_text, std::string &result);
    void StoreUserContext(const std::string &context);
    std::string GetUserContext() const;
    Task predictTaskFromInput(const std::string &input);
};

#endif // MODEL_RUNNER_H
