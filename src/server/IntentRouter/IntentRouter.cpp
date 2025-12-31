#include "IntentRouter.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <memory>
#include <utility>

#include <nlohmann/json.hpp>

using nlohmann::json;

IntentRouter::IntentRouter(std::string jsonPath)
    : path_(std::move(jsonPath))
{
    Reload();

    std::error_code ec;
    auto t = std::filesystem::last_write_time(path_, ec);
    if (!ec) lastWrite_ = t;
}

std::string IntentRouter::GetLastError() const
{
    std::lock_guard<std::mutex> lk(errorMutex_);
    return lastError_;
}

std::string IntentRouter::ToLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return s;
}

bool IntentRouter::ContainsAnyKeyword(const std::string& textLower,
                                      const std::vector<std::string>& kws)
{
    if (kws.empty()) return true;
    for (const auto& kw : kws)
    {
        if (!kw.empty() && textLower.find(kw) != std::string::npos)
            return true;
    }
    return false;
}

bool IntentRouter::Reload()
{
    std::ifstream f(path_);
    if (!f)
    {
        std::lock_guard<std::mutex> lk(errorMutex_);
        lastError_ = "IntentRouter: failed to open " + path_;
        return false;
    }

    json j;
    try
    {
        f >> j;
    }
    catch (const std::exception& e)
    {
        std::lock_guard<std::mutex> lk(errorMutex_);
        lastError_ = std::string("IntentRouter: JSON parse error: ") + e.what();
        return false;
    }

    if (!j.contains("intents") || !j["intents"].is_array())
    {
        std::lock_guard<std::mutex> lk(errorMutex_);
        lastError_ = "IntentRouter: missing key 'intents' (must be an array)";
        return false;
    }

    auto vec = std::make_shared<std::vector<IntentSpec>>();
    vec->reserve(j["intents"].size());

    try
    {
        for (const auto& ij : j["intents"])
        {
            IntentSpec intent;
            intent.label = ij.value("label", "");
            intent.priority = ij.value("priority", 0);

            for (const auto& kw : ij.value("any_keywords", std::vector<std::string>{}))
                intent.any_keywords.push_back(ToLower(kw));

            for (const auto& pattern : ij.value("regex", std::vector<std::string>{}))
                intent.regexes.emplace_back(pattern, std::regex::ECMAScript);

            if (!intent.label.empty())
                vec->push_back(std::move(intent));
        }
    }
    catch (const std::exception& e)
    {
        std::lock_guard<std::mutex> lk(errorMutex_);
        lastError_ = std::string("IntentRouter: error building intents: ") + e.what();
        return false;
    }

    std::sort(vec->begin(), vec->end(),
              [](const IntentSpec& a, const IntentSpec& b) {
                  return a.priority > b.priority;
              });

    std::atomic_store(&intents_, vec);

    {
        std::lock_guard<std::mutex> lk(errorMutex_);
        lastError_.clear();
    }

    return true;
}

void IntentRouter::ReloadIfChanged()
{

    std::error_code ec;
    auto t = std::filesystem::last_write_time(path_, ec);
    if (ec) return;

    std::lock_guard<std::mutex> lk(reloadMutex_);

    if (t == lastWrite_) return;

    if (Reload())
        lastWrite_ = t;
}

std::string IntentRouter::MatchIntent(const std::string& input) const
{
    auto intentsPtr = std::atomic_load(&intents_);
    if (!intentsPtr) return std::string("Unknown");

    const std::string textLower = ToLower(input);

    for (const auto& intent : *intentsPtr)
    {
        if (!ContainsAnyKeyword(textLower, intent.any_keywords))
            continue;

        bool regexHit = intent.regexes.empty();
        for (const auto& rx : intent.regexes)
        {
            if (std::regex_match(textLower, rx))
            {
                regexHit = true;
                break;
            }
        }
        if (!regexHit) continue;

        return intent.label;
    }

    return std::string("Unknown");
}
