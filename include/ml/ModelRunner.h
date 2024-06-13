#ifndef MODEL_RUNNER_H
#define MODEL_RUNNER_H

#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/model.h>
#include <string>
#include <vector>
#include "Task.h"

class ModelRunner
{
public:
    ModelRunner(const std::string &model_path);
    bool IsLoaded() const;
    bool RunInference(const std::string &input_text, std::vector<float> &result);
    Task predictTaskFromInput(const std::string &input);

    void LoadClassNames(const std::string &file_path);

private:
    std::unique_ptr<tflite::FlatBufferModel> model_;
    std::unique_ptr<tflite::Interpreter> interpreter_;
    std::vector<std::string> class_names_;
};

#endif // MODEL_RUNNER_H
