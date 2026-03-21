#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Task structure definition
typedef struct
{
    TaskHandle_t handle;
    TaskFunction_t function;
    uint32_t stackSize;
    void *parameters;
    UBaseType_t priority;
    const char *name;
} TaskStruct;

// Task list definition
typedef std::vector<TaskStruct> TaskList;

// Create a single task
static inline void createTask(TaskStruct *taskStruct) 
{
    BaseType_t core = (
        taskStruct->name == "Actuator" || 
        taskStruct->name == "Sensor"
    ) ? PRO_CPU_NUM : APP_CPU_NUM; 

    xTaskCreatePinnedToCore(
        taskStruct->function,
        taskStruct->name,
        taskStruct->stackSize,
        taskStruct->parameters,
        taskStruct->priority,
        &taskStruct->handle,
        core
    );

    // Halt if task creation fails
    if(taskStruct->handle == NULL) 
    {
        if(DEBUG_ON)
        {
            Serial.print("Failed to create task: ");
            Serial.println(taskStruct->name);
        }
        while(1); 
    }
}

// Create multiple tasks from a list
static inline void createTasks(TaskList &tasks) 
{
    for (auto &task : tasks)
    {
        createTask(&task);
        if(DEBUG_ON)
            Serial.printf("Task %s created with priority %d\n", task.name, task.priority);
    }
}


