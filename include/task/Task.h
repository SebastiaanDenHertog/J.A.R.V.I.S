#ifndef TASK_H_
#define TASK_H_

#include <string>
#include <vector>
#include "ClientInfo.h"
#include "DataStructures.h"

class Task
{
public:
    enum TaskType
    {
        // All the task types
        Book,
        Calculate,
        Calendar,
        Call,
        Connect,
        ControlHeating,
        ControlLight,
        Define,
        Email,
        Find,
        GetRecipe,
        GetShippingInfo,
        Locate,
        Message,
        Navigate,
        NewsQuery,
        OrderItem,
        PauseMusic,
        PauseVideo,
        PlayMusic,
        PlayVideo,
        Read,
        Recommend,
        ResumeVideo,
        SetAlarm,
        SetTimer,
        SetVolume,
        ShoppingList,
        Summarize,
        Translate,
        WeatherQuery,
        Info,
        ERROR
    };

    std::string description;
    int priority;
    TaskType type;
    ClientInfo device;
    std::string output;

    // Home Assistant specific fields
    std::string entityId;
    std::string service;
    std::string newState;
    std::vector<std::vector<std::string>> entities;

    // Add UserCommand as a member
    UserCommand userCommand;

    // Constructors
    Task(const std::string &description, int priority, const ClientInfo &device, TaskType type, UserCommand &userCommand, const std::vector<std::vector<std::string>> &entities = {});

    Task(const std::string &description, const std::string &entityId, const std::string &service, const std::string &newState, int priority, const ClientInfo &device, TaskType type,  UserCommand &userCommand, const std::vector<std::vector<std::string>> &entities = {});

private:
};

#endif // TASK_H_
