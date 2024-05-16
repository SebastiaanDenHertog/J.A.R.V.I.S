#ifndef MODEL_RUNNER_H
#define MODEL_RUNNER_H

#include <string>
#include <memory>
#include <vector>
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/interpreter.h"

class ModelRunner {
private:
    std::unique_ptr<tflite::FlatBufferModel> model_;
    std::unique_ptr<tflite::Interpreter> interpreter_;
    bool CreateInterpreter();

public:
    explicit ModelRunner(const std::string& model_path);
    bool IsLoaded() const;

    // Handle both float and array input
    float RunModel(float input_data);
    std::vector<float> RunModel(const std::vector<float>& input_data);
};

#endif // MODEL_RUNNER_H
