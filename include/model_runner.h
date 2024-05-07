#ifndef MODEL_RUNNER_H
#define MODEL_RUNNER_H

#include <string>
#include <memory>
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
    float RunModel(float input_data);
};

#endif // MODEL_RUNNER_H
