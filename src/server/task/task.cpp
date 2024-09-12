#include "Task.h"

Task::Task(const std::string &description, int priority, const ClientInfo &device, TaskType type, const std::vector<std::vector<std::string>> &entities)
    : description(description), priority(priority), device(device), type(type), entities(entities) {}

Task::Task(const std::string &description, const std::string &entityId, const std::string &service, const std::string &newState, int priority, const ClientInfo &device, TaskType type, const std::vector<std::vector<std::string>> &entities)
    : description(description), entityId(entityId), service(service), newState(newState), priority(priority), device(device), type(type), entities(entities) {}
