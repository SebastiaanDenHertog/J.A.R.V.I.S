/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    09-08-2024
 * @Date updated    29-09-2025
 * @Description     Constructor, destructor and methods for the TaskProcessor class
 **/

#include "TaskProcessor.h"
#include "MediaPlayer.h"
#include <iostream>
#include <random>

TaskProcessor::TaskProcessor(HomeAssistantAPI *homeAssistantAPI, ModelRunner &nerModel, ModelRunner &classificationModel)
    : nerModel_(nerModel),
      classificationModel_(classificationModel),
      homeAssistantAPI_(homeAssistantAPI)
{
    taskHandler_ = [this](const Task &task) {
        std::cout << "Handling task: " << task.description << std::endl;
    };
}

void TaskProcessor::processTask(const Task &task)
{
    if (task.description.empty()) {
        std::cout << "Received an empty task, ignoring." << std::endl;
        return;
    }

    switch (task.type)
    {
    case Task::Book:
        break;
    case Task::Calculate:
        break;
    case Task::Calendar:
        break;
    case Task::Call:
        break;
    case Task::Connect:
        break;
    case Task::ControlHeating:
        (void)processHomeAssistantTask(task);
        break;
    case Task::ControlLight:
        (void)processHomeAssistantTask(task);
        break;
    case Task::Define:
        break;
    case Task::Email:
        break;
    case Task::Find:
        break;
    case Task::GetRecipe:
        break;
    case Task::GetShippingInfo:
        break;
    case Task::Locate:
        break;
    case Task::Message:
        break;
    case Task::Navigate:
        break;
    case Task::NewsQuery:
        break;
    case Task::OrderItem:
        break;
    case Task::PauseMusic:
        break;
    case Task::PauseVideo:
        break;
    case Task::PlayMusic:
    {
        MediaPlayer player;
        player.setoutput(task.device, task.output);
        player.play(player.FindSong(task.entities));
        break;
    }
    case Task::PlayVideo:
        break;
    case Task::Read:
        break;
    case Task::Recommend:
        break;
    case Task::ResumeVideo:
        break;
    case Task::SetAlarm:
        break;
    case Task::SetTimer:
        break;
    case Task::SetVolume:
        break;
    case Task::ShoppingList:
        break;
    case Task::Summarize:
        break;
    case Task::Translate:
        break;
    case Task::WeatherQuery:
        break;
    case Task::ERROR:
        std::cerr << "Error task received: " << task.description << std::endl;
        break;
    case Task::Info:
        std::cout << "Info task: " << task.description << std::endl;
        break;
    default:
        std::cerr << "Unknown task type received: " << task.description << std::endl;
        break;
    }
}

bool TaskProcessor::processGeneralTask(const Task &task)
{
    std::cout << "Processing general task: " << task.description << std::endl;
    return true;
}

bool TaskProcessor::processHomeAssistantTask(const Task &task)
{
    std::cout << "Processing Home Assistant task: " << task.description << std::endl;

    if (!homeAssistantAPI_) {
        std::cerr << "Home Assistant API is not initialized." << std::endl;
        return false;
    }

    if (!task.service.empty()) {
        homeAssistantAPI_->callService("homeassistant", task.service, task.entityId);
        return true;
    }
    if (!task.newState.empty()) {
        homeAssistantAPI_->sendStateChange(task.entityId, task.newState);
        return true;
    }

    std::cerr << "No service or state provided for Home Assistant task." << std::endl;
    return false;
}

/**
 * @brief Checks if the task number is already in use; if free, inserts it.
 * @param taskNumber Reference to candidate number to check.
 * @return true if unique (and inserted), false if already in use.
 */
bool TaskProcessor::checkTaskNumber(int &taskNumber)
{
    std::lock_guard<std::mutex> lock(taskNumberMutex_);
    if (taskNumbers_.find(taskNumber) == taskNumbers_.end()) {
        taskNumbers_.insert(taskNumber);
        return true;
    }
    return false;
}

/**
 * @brief Creates a unique task number and reserves it.
 * @return The created task number.
 */
int TaskProcessor::createTaskNumber()
{
    static std::mt19937 rng{std::random_device{}()};
    static std::uniform_int_distribution<int> dist(0, 9999);

    for (;;) {
        int candidate = dist(rng);
        if (checkTaskNumber(candidate)) {
            return candidate;
        }
    }
}
