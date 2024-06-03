#ifndef MODEL_RUNNER_H
#define MODEL_RUNNER_H

#include <string>
#include <memory>
#include <vector>
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/interpreter.h"
#include "NetworkManager.h"
#ifdef SERVER_MODE
struct SoundData;
#endif

class ModelRunner
{
private:
    std::unique_ptr<tflite::FlatBufferModel> model_;
    std::unique_ptr<tflite::Interpreter> interpreter_;
    bool CreateInterpreter();

public:
#ifdef SERVER_MODE
    ModelRunner(const std::string &modelPath);
    void modelsLogic(SoundData *soundData);
#endif
    bool IsLoaded() const;
    float RunModel(float input_data);
    std::vector<float> RunModel(const std::vector<float> &input_data);
};

#endif // MODEL_RUNNER_H
