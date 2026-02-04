/**
 * modules.cpp
 * Implements the core logic for Scheduler, Producer-Consumer, and Banker's Algorithm.
 */

#include "os_sim.h"

// --- PART C: RESOURCE MANAGEMENT & DEADLOCK PREVENTION ---

// Initial System Resources (e.g., 10 units of R0, 5 of R1, 7 of R2)
void initialize_system_resources() {
    available_resources = {10, 5, 7}; 
}

// Banker's Algorithm Safety Check
bool bankers_is_safe(Process p) {
    // Check if Need <= Available
    for (int i = 0; i < RESOURCE_TYPES; i++) {
        if (p.need[i] > available_resources[i]) {
            return false; // Unsafe to allocate immediately
        }
    }
    return true;
}

// --- PART B: PRODUCER-CONSUMER MODULE ---

// Producer Thread: Generates processes and adds to buffer
void* producer_thread(void* arg) {
    int producer_id = *(int*)arg;
    while (simulation_running) {
        Process p;
        p.id = rand() % 1000 + 1;
        p.burst_time = rand() % 10 + 1;
        p.remaining_time = p.burst_time;
        p.priority = rand() % 5; // 0 (High) to 4 (Low)
        p.arrival_time = time(NULL);

        // Generate Random Resource Needs
        for(int i=0; i<RESOURCE_TYPES; i++) {
            p.max_need[i] = rand() % 4 + 1; // Needs 1-4 resources
            p.allocated[i] = 0;             // Initially 0 allocated
            p.need[i] = p.max_need[i];
        }

        // Wait for empty slot (sem_wait)
        sem_wait(&empty_slots);
        // Lock mutex
        pthread_mutex_lock(&mutex_lock);

        // Critical Section
        buffer.push(p);
        cout << "[Producer " << producer_id << "] Created Process P" << p.id 
             << " (Burst: " << p.burst_time << ", Prio: " << p.priority << ")" << endl;

        pthread_mutex_unlock(&mutex_lock);
        sem_post(&full_slots); // Signal full slot

        sleep(rand() % 3 + 1); // Simulate work
    }
    return NULL;
}

// Consumer Thread: Fetches from buffer and sends to Scheduler
void* consumer_thread(void* arg) {
    while (simulation_running) {
        // Wait for full slot (no busy waiting)
        sem_wait(&full_slots);
        pthread_mutex_lock(&mutex_lock);

        // Critical Section
        if (buffer.empty()) {
            pthread_mutex_unlock(&mutex_lock);
            continue;
        }

        Process p = buffer.front();
        buffer.pop();

        // Perform Deadlock Safety Check BEFORE adding to ready queue
        if (bankers_is_safe(p)) {
            // Allocate resources logically
            for(int i=0; i<RESOURCE_TYPES; i++) {
                available_resources[i] -= p.need[i];
                p.allocated[i] += p.need[i];
                p.need[i] = 0;
            }
            ready_queue.push_back(p);
            cout << "   [Consumer] Process P" << p.id << " is SAFE -> Ready Queue." << endl;
        } else {
            blocked_queue.push_back(p);
            cout << "   [Consumer] Process P" << p.id << " is UNSAFE -> Blocked." << endl;
        }

        pthread_mutex_unlock(&mutex_lock);
        sem_post(&empty_slots);

        sleep(1); // Simulate consumption time
    }
    return NULL;
}

// --- PART A: SCHEDULER MODULE ---

void run_scheduler() {
    if (ready_queue.empty()) {
        cout << "Scheduler: No ready processes." << endl;
        return;
    }

    cout << "\n--- STARTING SCHEDULER ---" << endl;
    cout << "Processes in Ready Queue: " << ready_queue.size() << endl;

    // Decision Rule
    if (ready_queue.size() <= 5) {
        cout << "Condition: <= 5 Processes. Using PRIORITY SCHEDULING." << endl;
        priority_scheduling();
    } else {
        cout << "Condition: > 5 Processes. Using ROUND ROBIN SCHEDULING." << endl;
        round_robin_scheduling();
    }
}

// Helper to sort by priority (Ascending: 0 is highest)
bool comparePriority(const Process &a, const Process &b) {
    return a.priority < b.priority; 
}

void priority_scheduling() {
    // Sort ready queue by priority
    vector<Process> local_queue = ready_queue;
    sort(local_queue.begin(), local_queue.end(), comparePriority);
    
    vector<int> exec_order;
    int current_time = 0;
    float total_wait = 0, total_turnaround = 0;

    cout << "\nPID\tPrio\tBurst\tWait\tTAT" << endl;

    for (const auto& p : local_queue) {
        exec_order.push_back(p.id);
        
        int wait_time = current_time; 
        int turnaround_time = wait_time + p.burst_time;
        current_time += p.burst_time;

        total_wait += wait_time;
        total_turnaround += turnaround_time;

        cout << "P" << p.id << "\t" << p.priority << "\t" << p.burst_time 
             << "\t" << wait_time << "\t" << turnaround_time << endl;
    }

    if (!local_queue.empty()) {
        cout << "\nAvg Wait: " << total_wait / local_queue.size() << endl;
    }
    print_gantt_chart(exec_order);
    
    // Clear ready queue after simulated run and reclaim resources
    for(const auto& p : local_queue) {
         for(int i=0; i<RESOURCE_TYPES; i++) available_resources[i] += p.max_need[i];
    }
    ready_queue.clear();
}

void round_robin_scheduling() {
    vector<Process> local_queue = ready_queue;
    vector<int> exec_order;
    int current_time = 0;
    int completed = 0;
    int n = local_queue.size();

    // Round Robin Logic
    while (completed != n) {
        bool work_done = false;
        for (int i = 0; i < n; i++) {
            if (local_queue[i].remaining_time > 0) {
                work_done = true;
                exec_order.push_back(local_queue[i].id);

                if (local_queue[i].remaining_time > TIME_QUANTUM) {
                    current_time += TIME_QUANTUM;
                    local_queue[i].remaining_time -= TIME_QUANTUM;
                } else {
                    current_time += local_queue[i].remaining_time;
                    local_queue[i].remaining_time = 0;
                    completed++;
                }
            }
        }
        if (!work_done) break;
    }

    cout << "Round Robin Simulation Complete." << endl;
    print_gantt_chart(exec_order);
    
    // Reclaim resources
    for(const auto& p : ready_queue) {
         for(int i=0; i<RESOURCE_TYPES; i++) available_resources[i] += p.max_need[i];
    }
    ready_queue.clear();
}

void print_gantt_chart(const vector<int>& execution_order) {
    cout << "\n[ GANTT CHART ]" << endl;
    cout << "| ";
    for (int pid : execution_order) {
        cout << "P" << pid << " | ";
    }
    cout << "\n---------------------------------" << endl;
}