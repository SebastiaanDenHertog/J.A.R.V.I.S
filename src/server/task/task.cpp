/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    06-06-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the Task class
 **/

#include "Task.h"

Task::Task(const std::string &description, int priority, const ClientInfo &device, TaskType type, UserCommand &userCommand, const std::vector<std::vector<std::string>> &entities)
    : description(description), priority(priority), device(device), type(type), userCommand(userCommand), entities(entities) {}

Task::Task(const std::string &description, const std::string &entityId, const std::string &service, const std::string &newState, int priority, const ClientInfo &device, TaskType type, UserCommand &userCommand, const std::vector<std::vector<std::string>> &entities)
    : description(description), entityId(entityId), service(service), newState(newState), priority(priority), device(device), type(type), userCommand(userCommand), entities(entities) {}
