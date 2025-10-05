/**
* @Authors         Sebastiaan den Hertog
 * @Date created    06-07-2024
 * @Date updated    29-09-2025
 * @Description     Constructor, destructor and methods for the TaskProcessor class
 */

#ifndef TASKPROCESSOR_H
#define TASKPROCESSOR_H

#include <functional>
#include <mutex>
#include <unordered_set>
#include "Task.h"
#include "HomeAssistantAPI.h"
#include "ModelRunner.h"

class TaskProcessor
{
public:
    TaskProcessor(HomeAssistantAPI *homeAssistantAPI, ModelRunner &nerModel, ModelRunner &classificationModel);
    void processTask(const Task &task);
    int  createTaskNumber();

private:
    bool processGeneralTask(const Task &task);
    bool processHomeAssistantTask(const Task &task);
    ModelRunner      &nerModel_;
    ModelRunner      &classificationModel_;
    HomeAssistantAPI *homeAssistantAPI_{nullptr};
    std::mutex                       taskNumberMutex_;
    std::unordered_set<int>          taskNumbers_;
    bool checkTaskNumber(int &taskNumber);
    std::function<void(const Task &task)> taskHandler_;
};

#endif // TASKPROCESSOR_H