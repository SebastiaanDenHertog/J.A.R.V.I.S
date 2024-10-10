/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    06-07-2024
 * @Date updated    04-10-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the TaskProcessor class
 */

#ifndef TASKPROCESSOR_H
#define TASKPROCESSOR_H

#include <functional>
#include "Task.h"
#include "HomeAssistantAPI.h"
#include "ModelRunner.h"

class TaskProcessor
{
public:
    TaskProcessor(HomeAssistantAPI *homeAssistantAPI, ModelRunner &nerModel, ModelRunner &classificationModel);
    void processTask(const Task &task);

private:
    std::function<void(const Task &task)> taskHandler_;
    bool processGeneralTask(const Task &task);
    bool processHomeAssistantTask(const Task &task);
    ModelRunner &nerModel_;
    ModelRunner &classificationModel_;
    HomeAssistantAPI *homeAssistantAPI_;
};

#endif