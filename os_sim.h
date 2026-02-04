/**
 * os_sim.h
 * Shared definitions, structures, and function prototypes.
 * Standard namespace is used globally here for simplicity.
/**
 * os_sim.h
 * Shared definitions, structures, and function prototypes.
 */

#ifndef OS_SIM_H
#define OS_SIM_H

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm> // for sort
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>  // for sleep
#include <cstdlib>   // for rand
#include <ctime>     // for time

using namespace std;

// --- CONSTANTS ---
#define RESOURCE_TYPES 3
#define BUFFER_SIZE 5
#define TIME_QUANTUM 3 // For Round Robin

// --- STRUCTURES ---
struct Process {
    int id;
    int burst_time;
    int remaining_time; // For Round Robin
    int priority;       // Lower # = Higher Priority
    int arrival_time;
    
    // Resource Management for Banker's Algo
    int max_need[RESOURCE_TYPES];
    int allocated[RESOURCE_TYPES];
    int need[RESOURCE_TYPES];
};

// --- GLOBAL SHARED VARIABLES (extern) ---
extern vector<int> available_resources;
extern queue<Process> buffer;      // The Bounded Buffer
extern vector<Process> ready_queue; // Processes that passed Banker's check
extern vector<Process> blocked_queue; // Processes that failed Banker's check

// Synchronization Primitives
extern sem_t empty_slots;
extern sem_t full_slots;
extern pthread_mutex_t mutex_lock;

// Simulation Control
extern bool simulation_running;

// --- FUNCTION PROTOTYPES ---

// Part A: Scheduler
void run_scheduler();
void priority_scheduling();
void round_robin_scheduling();
void print_gantt_chart(const vector<int>& execution_order);

// Part B: Producer-Consumer
void* producer_thread(void* arg);
void* consumer_thread(void* arg);

// Part C: Deadlock Prevention
bool bankers_is_safe(Process p);
void initialize_system_resources();

#endif