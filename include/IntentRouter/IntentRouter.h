/**
* @Authors         Sebastiaan den Hertog
 * @Date created    31-12-2025
 * @Date updated    31-12-2025
 * @Description     Header file for the IntentRouter class
 */

#ifndef INTENTROUTER_H
#define INTENTROUTER_H

#include <filesystem>
#include <memory>
#include <mutex>
#include <regex>
#include <string>
#include <vector>

struct IntentSpec
{
    std::string label;
    int priority = 0;
    std::vector<std::string> any_keywords;
    std::vector<std::regex> regexes;
};

class IntentRouter
{
public:
    explicit IntentRouter(std::string jsonPath);
    bool Reload();
    void ReloadIfChanged();
    std::string MatchIntent(const std::string& input) const;
    const std::string& GetPath() const { return path_; }
    std::string GetLastError() const;

private:
    static std::string ToLower(std::string s);
    static bool ContainsAnyKeyword(const std::string& textLower,
                                   const std::vector<std::string>& kws);
    std::string path_;
    mutable std::mutex reloadMutex_;
    std::filesystem::file_time_type lastWrite_{};
    std::shared_ptr<std::vector<IntentSpec>> intents_{nullptr};

    mutable std::mutex errorMutex_;
    std::string lastError_;
};
#endif