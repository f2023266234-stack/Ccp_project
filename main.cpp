/**
 * main.cpp
 * Entry point. Initializes threads and provides the CLI Menu.
 */

#include "os_sim.h"

// Define Shared Variables
vector<int> available_resources;
queue<Process> buffer;
vector<Process> ready_queue;
vector<Process> blocked_queue;
sem_t empty_slots;
sem_t full_slots;
pthread_mutex_t mutex_lock;
bool simulation_running = true;

// Thread Handles
pthread_t prod1, prod2, cons;
int id1 = 1, id2 = 2;

void display_system_state() {
    cout << "\n--- CURRENT SYSTEM STATE ---" << endl;
    cout << "Buffer (Waiting to be consumed): " << buffer.size() << "/" << BUFFER_SIZE << endl;
    cout << "Ready Queue (Passed Banker's): " << ready_queue.size() << endl;
    cout << "Blocked Queue (Unsafe): " << blocked_queue.size() << endl;
    
    if (!available_resources.empty()) {
        cout << "Available Resources: [" 
             << available_resources[0] << ", " 
             << available_resources[1] << ", " 
             << available_resources[2] << "]" << endl;
    }
}

int main() {
    // 1. Initialization
    srand(time(0));
    initialize_system_resources();
    sem_init(&empty_slots, 0, BUFFER_SIZE);
    sem_init(&full_slots, 0, 0);
    pthread_mutex_init(&mutex_lock, NULL);

    cout << "=== MINI OS SIMULATOR CCP ===" << endl;

    // 2. Start Worker Threads
    pthread_create(&prod1, NULL, producer_thread, &id1);
    pthread_create(&prod2, NULL, producer_thread, &id2);
    pthread_create(&cons, NULL, consumer_thread, NULL);

    // 3. Menu Interface
    int choice;
    while (true) {
        cout << "\nMAIN MENU" << endl;
        cout << "1. Display System State" << endl;
        cout << "2. Run Scheduler (Process Ready Queue)" << endl;
        cout << "3. Force Add Process (Manual)" << endl;
        cout << "4. Exit" << endl;
        cout << "Select: ";
        cin >> choice;

        switch (choice) {
            case 1:
                display_system_state();
                break;
            case 2:
                // Lock mutex to prevent consumer from modifying queue while scheduling
                pthread_mutex_lock(&mutex_lock);
                run_scheduler();
                pthread_mutex_unlock(&mutex_lock);
                break;
            case 3:
                {
                    // Manual Add
                    Process p;
                    p.id = 999; p.burst_time = 5; p.priority = 1;
                    p.remaining_time = 5;
                    p.arrival_time = time(NULL);
                    for(int i=0; i<3; i++) { p.max_need[i]=1; p.need[i]=1; p.allocated[i]=0; }
                    
                    pthread_mutex_lock(&mutex_lock);
                    buffer.push(p); 
                    pthread_mutex_unlock(&mutex_lock);
                    sem_post(&full_slots); // Signal consumer
                    cout << "Manual Process P999 added to buffer." << endl;
                }
                break;
            case 4:
                simulation_running = false;
                // Cleanup to ensure threads exit
                sem_post(&empty_slots); sem_post(&full_slots); 
                pthread_cancel(prod1); pthread_cancel(prod2); pthread_cancel(cons);
                sem_destroy(&empty_slots);
                sem_destroy(&full_slots);
                pthread_mutex_destroy(&mutex_lock);
                cout << "Exiting Simulator." << endl;
                return 0;
            default:
                cout << "Invalid choice." << endl;
        }
    }
    return 0;
}