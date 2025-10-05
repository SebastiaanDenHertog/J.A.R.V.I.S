/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    13-06-2024
 * @Date updated    17-09-2025
 * @Description     Constructor, destructor and methods for the ModelRunner class
 *                  (TensorFlow SavedModel C++ backend)
 */

#ifndef MODEL_RUNNER_H
#define MODEL_RUNNER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

// TensorFlow C++ (SavedModel + Session)
#include <tensorflow/cc/saved_model/loader.h>
#include <tensorflow/core/framework/tensor.h>
#include <tensorflow/core/public/session.h>

class ModelRunner
{
public:
    // model_dir: path to a TF SavedModel directory (with saved_model.pb / saved_model.pbtxt)
    // input_op: full input tensor name (e.g. "serving_default_input_ids:0")
    // output_op: full output tensor name (e.g. "StatefulPartitionedCall:0" or "Identity:0")
    explicit ModelRunner(const std::string& model_dir,
                         const std::string& input_op,
                         const std::string& output_op);

    // Load/ready checks
    bool IsLoaded() const;

    // Tokenizer & labels
    void LoadTokenizer(const std::string& tokenizer_path);  // expects the same JSON schema you used
    void LoadLabels(const std::string& labels_path);        // id->label JSON as before

    // Inference utilities
    bool RunInference(const std::string& input_text,
                      std::vector<std::vector<float>>& result);

    std::pair<std::string, std::vector<std::string>>
    PredictlabelFromInput(const std::string& input);

    std::string ClassifySentence(const std::string& input);

    // Optional: allow changing op names after construction (useful when exporting different graphs)
    void SetInputOp(const std::string& input_op) { input_op_ = input_op; }
    void SetOutputOp(const std::string& output_op) { output_op_ = output_op; }

private:
    // Simple whitespace tokenizer backed by loaded word_index
    std::vector<int> TokenizeInput(const std::string& input_text);

    // SavedModel bundle and session
    tensorflow::SavedModelBundleLite bundle_;
    tensorflow::Session* session_{nullptr};

    // IO op names
    std::string input_op_;
    std::string output_op_;

    // Metadata
    std::unordered_map<int, std::string> labels_;
    std::unordered_map<int, std::string> tokenizer_index_word_;
    std::unordered_map<std::string, int> tokenizer_word_index_;
    int max_length_{0};

    // Internal state
    bool loaded_{false};
};

#endif // MODEL_RUNNER_H
