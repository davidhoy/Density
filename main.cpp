//
//  main.cpp
//  SimpleDaemon
//
//  Created by David Hoy on 12/18/19.
//  Copyright © 2019 David Hoy. All rights reserved.
//

#include <stdlib.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <queue>
#include <string>
#include <mutex>
#include <random>
using namespace std;

#define RUN_AS_DAEMON

class Event
{
public:
    int         m_eventId;
    int         m_timeElapsed;
    std::string m_msg;
    
    Event();
    Event(int eventId, int timeElapsed, std::string msg) {
        m_eventId     = eventId;
        m_timeElapsed = timeElapsed;
        m_msg         = msg;
    }
};

class Daemon
{
    std::mutex m_mutex;
    std::queue<shared_ptr<Event>> m_queue;
    bool m_isRunning;

public:
    /*
     * Class constructor
     */
    Daemon() {
        m_isRunning = false;
    }
    
    /*
     * Starts the daemon tasks
     */
    void start() {
        std::thread thread1(&Daemon::mainTask, this);
        std::thread thread2(&Daemon::eventTask, this);
        
        m_isRunning = true;
        
        thread2.join();
        thread1.join();
        
        m_isRunning = false;
    }
    
    bool isRunning() { return m_isRunning; }
 
    /*
     * Event task - generates psuedo-random events and sends them to the main task
     */
    void eventTask() {
        // Set up to generate random integers between 1 and 5
        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution(1,5);
        
        int eventId = 0;
        
        while (1) {
            // Make thread sleep for a random period between 1 and 5 seconds
            int ms = distribution(generator) * 1000;
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
       
            std::cout << "Generate psuedo-async event" <<std::endl;
            std::lock_guard<std::mutex> lock(m_mutex);
            
            auto msg = std::string("Noooobody expects the Spanish Inquisition...");
            auto event = make_shared<Event>(eventId, ms, msg);
            m_queue.push(event);
            
            eventId++;
        }
    }
    
    /*
     * Main task for the daemon - monitors the queue for events, and processes them
     */
    void mainTask() {
        while (true) {
            while (m_queue.empty() == false) {
                std::lock_guard<std::mutex> lock(m_mutex);
                auto event = m_queue.front();
                m_queue.pop();
                
                std::cout << "Handle event " << event->m_eventId <<
                                     ", ms=" << event->m_timeElapsed <<
                                    ", msg=" << event->m_msg << endl;
            }
        }
    }
};



/*
 * Main entry point
 */
int main(int argc, char** argv) {
    
    Daemon daemon;
    std::thread thread(&Daemon::start, &daemon);
    
#ifdef RUN_AS_DAEMON
    thread.detach();
#else
    thread.join();
    while (daemon.isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
#endif

    return 0;
}




