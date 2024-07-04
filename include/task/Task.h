#ifndef TASK_H
#define TASK_H

#include <string>
#include <vector>
#include "ClientInfo.h"

class Task
{
public:
    enum TaskType
    {
        ControlLight,
        SetTimer,
        PlayMusic,
        SetReminder,
        WeatherQuery,
        SetAlarm,
        SetTemperature,
        GetNews,
        Lock,
        Open,
        Close,
        Start,
        Stop,
        Pause,
        Resume,
        TurnOn,
        TurnOff,
        Check,
        Read,
        Find,
        Locate,
        Book,
        Call,
        Message,
        Water,
        Question,
        Command,
        Information,
        SetVolume,
        GetShippingInfo,
        ReadMessages,
        GetRecipe,
        Connect,
        PlayVideo,
        ShoppingList,
        Calendar,
        Info,
        GetTraffic,
        OrderItem,
        SetMachine,
        SentMessage,
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

    Task(const std::string &description, int priority, const ClientInfo &device, TaskType type, const std::vector<std::vector<std::string>> &entities = {});

    Task(const std::string &description, const std::string &entityId, const std::string &service, const std::string &newState, int priority, const ClientInfo &device, TaskType type, const std::vector<std::vector<std::string>> &entities = {});

private:
};

#endif // TASK_H
