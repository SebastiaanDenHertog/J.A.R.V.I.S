/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    06-06-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the Task class
 */

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
        Ner,
        Classify,
        Book,
        Browse,
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
    int taskNumber;
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
    Task(int taskNumber, const std::string &description, int priority, const ClientInfo &device, TaskType type, UserCommand &userCommand, const std::vector<std::vector<std::string>> &entities = {});

    Task(int taskNumber, const std::string &description, const std::string &entityId, const std::string &service, const std::string &newState, int priority, const ClientInfo &device, TaskType type, UserCommand &userCommand, const std::vector<std::vector<std::string>> &entities = {});
    TaskType stringToTaskType(const std::string &str);

    int get_task_number() const {
        return taskNumber;
    }

    void set_task_number(int task_number) {
        taskNumber = task_number;
    }

    std::string get_description() const {
        return description;
    }

    void set_description(const std::string &description);

    int get_priority() const {
        return priority;
    }

    void set_priority(int priority) {
        this->priority = priority;
    }

    TaskType get_type() const {
        return type;
    }

    void set_type(TaskType type) {
        this->type = type;
    }

    ClientInfo get_device() const {
        return device;
    }

    void set_device(const ClientInfo &device) {
        this->device = device;
    }

    std::string get_output() const {
        return output;
    }

    void set_output(const std::string &output) {
        this->output = output;
    }

    std::string get_entity_id() const {
        return entityId;
    }

    void set_entity_id(const std::string &entity_id) {
        entityId = entity_id;
    }

    std::string get_service() const {
        return service;
    }

    void set_service(const std::string &service) {
        this->service = service;
    }

    std::string get_new_state() const {
        return newState;
    }

    void set_new_state(const std::string &new_state) {
        newState = new_state;
    }

    std::vector<std::vector<std::string>> get_entities() const {
        return entities;
    }

    void set_entities(const std::vector<std::vector<std::string>> &entities) {
        this->entities = entities;
    }

    UserCommand get_user_command() const {
        return userCommand;
    }

    void set_user_command(const UserCommand &user_command) {
        userCommand = user_command;
    }
};

#endif // TASK_H_
