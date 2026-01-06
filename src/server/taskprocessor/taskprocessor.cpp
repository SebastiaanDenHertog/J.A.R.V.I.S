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

TaskProcessor::TaskProcessor(HomeAssistantAPI *homeAssistantAPI, ModelRunner &nerModel, ModelRunner &classificationModel, IntentRouter &intentRouter, InputHandler &inputHandler,
TaskProcessor &taskProcessor):
    nerModel_(nerModel),
    classificationModel_(classificationModel),
    intentRouter_(intentRouter),
    homeAssistantAPI_(homeAssistantAPI),
    inputHandler_(inputHandler),
    taskProcessor_(taskProcessor)
{
    taskHandler_ = [](const Task &task) {
        std::cout << "Handling task: " << task.description << std::endl;
    };
}

/**
 * @brief Processes a task by executing the corresponding actions based on its type.
 * @param task Reference to the task object containing the type and additional information.
 *
 * This function first validates the task description, ignoring empty tasks. Depending on the type of
 * the task, it performs appropriate actions such as parsing the task for named entities, updating
 * user commands, managing tasks in the input handler, invoking sub-processors, or directly handling
 * specific task types. For unrecognized or erroneous task types, it logs the issue and skips further
 * processing.
 */

void TaskProcessor::processTask(Task &task)
{
    if (task.description.empty()) {
        std::cout << "Received an empty task, ignoring." << std::endl;
        return;
    }
    switch (task.type)
    {
        case Task::Ner:
        {
            auto [_, predicted_entities] = nerModel_.PredictlabelFromInput(task.description); // Ignore the first part
            std::vector<std::pair<std::string, std::string>> sentence_entities;
            std::istringstream iss(task.description);
            std::string word;
            unsigned long entity_index = 0;
            while (iss >> word && entity_index < predicted_entities.size())
            {
                sentence_entities.emplace_back(word, predicted_entities[entity_index]);
                entity_index++;
            }
            std::cout << "Sentence and Entities: " << std::endl;
            for (const auto &pair : sentence_entities)
            {
                std::cout << "Word: " << pair.first << " -> Entity: " << pair.second << std::endl;
            }
            UserCommand user_command(task.description, sentence_entities, nullptr, predicted_entities);
            task.set_user_command(user_command);
            task.set_type(Task::Classify);
            inputHandler_.addTask(task);
            taskProcessor_.processTask(task);
            break;

        }

        case Task::Classify:
        {
            intentRouter_.ReloadIfChanged();
            std::string label = intentRouter_.MatchIntent(task.description);
            UserCommand user_command(user_command.user_input, user_command.sentence_entities, label, user_command.predicted_entities);
            task.set_user_command(user_command);
            task.set_type(task.stringToTaskType(label));
            responseReturn(task);
            break;
        }
    case Task::Book:
            responseReturn(task);
        break;
    case Task::Calculate:
            responseReturn(task);
        break;
    case Task::Calendar:
            responseReturn(task);
        break;
    case Task::Call:
            responseReturn(task);
        break;
    case Task::Connect:
            responseReturn(task);
        break;
    case Task::ControlHeating:
        (void)processHomeAssistantTask(task);
            responseReturn(task);
        break;
    case Task::ControlLight:
        (void)processHomeAssistantTask(task);
            responseReturn(task);
        break;
    case Task::Define:
            responseReturn(task);
        break;
    case Task::Email:
            responseReturn(task);
        break;
    case Task::Find:
            responseReturn(task);
        break;
    case Task::GetRecipe:
            responseReturn(task);
        break;
    case Task::GetShippingInfo:
            responseReturn(task);
        break;
    case Task::Locate:
            responseReturn(task);
        break;
    case Task::Message:
            responseReturn(task);
        break;
    case Task::Navigate:
            responseReturn(task);
        break;
    case Task::NewsQuery:
            responseReturn(task);
        break;
    case Task::OrderItem:
            responseReturn(task);
        break;
    case Task::PauseMusic:
            responseReturn(task);
        break;
    case Task::PauseVideo:
            responseReturn(task);
        break;
    case Task::PlayMusic:
    {
        MediaPlayer player;
        player.setoutput(task.device, task.output);
        player.play(player.FindSong(task.entities));
        responseReturn(task);
        break;
    }
    case Task::PlayVideo:
            responseReturn(task);
        break;
    case Task::Read:
            responseReturn(task);
        break;
    case Task::Recommend:
            responseReturn(task);
        break;
    case Task::ResumeVideo:
            responseReturn(task);
        break;
    case Task::SetAlarm:
            responseReturn(task);
        break;
    case Task::SetTimer:
            responseReturn(task);
        break;
    case Task::SetVolume:
            responseReturn(task);
        break;
    case Task::ShoppingList:
            responseReturn(task);
        break;
    case Task::Summarize:
            responseReturn(task);
        break;
    case Task::Translate:
            responseReturn(task);
        break;
    case Task::WeatherQuery:
            responseReturn(task);
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

void TaskProcessor::responseReturn(const Task &task)
{
    std::cout << "Returning response for task: " << task.description << std::endl;
}
