#ifndef TASK_H
#define TASK_H

#include <string>

class Task
{
public:
    enum TaskType
    {
        GENERAL,
        HOME_ASSISTANT,
        ERROR
    };

    std::string description;
    int priority;
    TaskType type;

    // Home Assistant specific fields
    std::string entityId;
    std::string service;
    std::string newState;

    Task(const std::string &description, int priority, TaskType type = GENERAL);

    // Constructor for Home Assistant tasks
    Task(const std::string &description, const std::string &entityId, const std::string &service, const std::string &newState, int priority = 1);
};

#endif // TASK_H
