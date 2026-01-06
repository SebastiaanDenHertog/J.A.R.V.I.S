/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    06-06-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the Task class
 **/

#include "Task.h"

#include <unordered_map>

Task::Task(int taskNumber, const std::string &description, int priority, const ClientInfo &device, TaskType type, UserCommand &userCommand, const std::vector<std::vector<std::string>> &entities)
    : taskNumber(taskNumber), description(description), priority(priority), device(device), type(type), userCommand(userCommand), entities(entities) {}

Task::Task(int taskNumber, const std::string &description, const std::string &entityId, const std::string &service, const std::string &newState, int priority, const ClientInfo &device, TaskType type, UserCommand &userCommand, const std::vector<std::vector<std::string>> &entities)
    : taskNumber(taskNumber), description(description), entityId(entityId), service(service), newState(newState), priority(priority), device(device), type(type), userCommand(userCommand), entities(entities) {}

/**
 * @brief Convert a string to the corresponding Task::TaskType enum.
 * @param str The string representation of the task type.
 * @return Corresponding Task::TaskType enum value.
 */
Task::TaskType Task::stringToTaskType(const std::string &str)
{
    static const std::unordered_map<std::string, Task::TaskType> strToTaskType = {
        {"Book", Task::Book},// booking a reservation
        {"Browse", Task::Browse},
        {"Calculate", Task::Calculate},
        {"Calendar", Task::Calendar},
        {"Call", Task::Call},
        {"Connect", Task::Connect},
        {"ControlHeating", Task::ControlHeating},
        {"ControlLight", Task::ControlLight},
        {"Define", Task::Define},
        {"Email", Task::Email},
        {"Find", Task::Find},
        {"GetRecipe", Task::GetRecipe},
        {"GetShippingInfo", Task::GetShippingInfo},
        {"Locate", Task::Locate},
        {"Message", Task::Message},
        {"Navigate", Task::Navigate},
        {"NewsQuery", Task::NewsQuery},
        {"OrderItem", Task::OrderItem},
        {"PauseMusic", Task::PauseMusic},
        {"PauseVideo", Task::PauseVideo},
        {"PlayMusic", Task::PlayMusic},
        {"PlayVideo", Task::PlayVideo},
        {"Read", Task::Read},
        {"Recommend", Task::Recommend},
        {"ResumeVideo", Task::ResumeVideo},
        {"SetAlarm", Task::SetAlarm},
        {"SetTimer", Task::SetTimer},
        {"SetVolume", Task::SetVolume},
        {"ShoppingList", Task::ShoppingList},
        {"Summarize", Task::Summarize},
        {"Translate", Task::Translate},
        {"WeatherQuery", Task::WeatherQuery}};
    auto it = strToTaskType.find(str);
    return it != strToTaskType.end() ? it->second : Task::ERROR;
}