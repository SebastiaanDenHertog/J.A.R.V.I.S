/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    13-06-2024
 * @Date updated    03-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the ModelRunner class
 */

#ifndef MODEL_RUNNER_H
#define MODEL_RUNNER_H

#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/model.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/op_resolver.h>
#include <tensorflow/lite/tools/command_line_flags.h>
#include <tensorflow/lite/tools/list_flex_ops.h>
#include <string>
#include <vector>
#include <unordered_map>

class ModelRunner
{
public:
    ModelRunner(const std::string &model_path);
    bool IsLoaded() const;
    void LoadTokenizer(const std::string &tokenizer_path);
    void LoadLabels(const std::string &labels_path);
    bool RunInference(const std::string &input_text, std::vector<std::vector<float>> &result);
    std::pair<std::string, std::vector<std::string>> PredictlabelFromInput(const std::string &input);
    std::string ClassifySentence(const std::string &input);

private:
    std::vector<int> TokenizeInput(const std::string &input_text);

    std::unique_ptr<tflite::FlatBufferModel> model_;
    std::unique_ptr<tflite::Interpreter> interpreter_;
    std::unordered_map<int, std::string> labels_;
    std::unordered_map<int, std::string> tokenizer_index_word_;
    std::unordered_map<std::string, int> tokenizer_word_index_;
    int max_length_;
};

#endif // MODEL_RUNNER_H