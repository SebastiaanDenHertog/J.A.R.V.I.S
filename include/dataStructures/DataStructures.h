#include <string>
#include <vector>
#include <utility> 

struct UserCommand
{
    std::string user_input;                                          // The input command from the user
    std::vector<std::pair<std::string, std::string>> sentence_entities;  // Word-entity pairs
    std::string intent_label;                                        // Predicted intent
    std::vector<std::string> predicted_entities;                     // List of predicted entities

    UserCommand(const std::string& input,
                const std::vector<std::pair<std::string, std::string>>& entities,
                const std::string& label,
                const std::vector<std::string>& predicted_ents)
        : user_input(input),
          sentence_entities(entities),
          intent_label(label),
          predicted_entities(predicted_ents) {}
};
