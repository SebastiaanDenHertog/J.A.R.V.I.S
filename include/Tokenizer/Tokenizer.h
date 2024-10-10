/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    13-07-2024
 * @Date updated    13-07-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the Tokenizer class
 **/

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>

class Tokenizer
{
public:
    Tokenizer()
    {
        word_index_["<OOV>"] = 1;
    }

    void fit_on_texts(const std::vector<std::string> &texts)
    {
        int index = 2; // Start indexing from 2 as 1 is for OOV
        for (const auto &text : texts)
        {
            std::istringstream iss(text);
            std::string word;
            while (iss >> word)
            {
                if (word_index_.find(word) == word_index_.end())
                {
                    word_index_[word] = index++;
                }
            }
        }
    }

    std::vector<int> texts_to_sequences(const std::string &text)
    {
        std::vector<int> sequence;
        std::istringstream iss(text);
        std::string word;
        while (iss >> word)
        {
            if (word_index_.find(word) != word_index_.end())
            {
                sequence.push_back(word_index_[word]);
            }
            else
            {
                sequence.push_back(word_index_["<OOV>"]);
            }
        }
        return sequence;
    }

private:
    std::unordered_map<std::string, int> word_index_;
};

#endif // TOKENIZER_H
