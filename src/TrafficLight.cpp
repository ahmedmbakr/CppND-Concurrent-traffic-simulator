#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> uLock(_mtx);

    _condition_variable.wait(uLock, [this]{ return !this->_queue.empty(); });

    T message = std::move(_queue.front());
    _queue.pop_front();
    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    //TODO: may need to add a sleep for a small time
    //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
    //perform modifications under the lock
    std::lock_guard<std::mutex> ulock(_mtx);

    _queue.push_back(std::move(msg));

    _condition_variable.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto trafficPhase = _messageQueue.receive();
        if(trafficPhase == TrafficLightPhase::green)
        {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this)); 
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    std::cout<< "enter cycleThroughPhases\n";
    // Seed with a real random value, if available
    std::random_device r;
 
    // Choose a random mean between 1 and 6
    std::default_random_engine e1(r());
    std::uniform_int_distribution<int> uniform_dist(4000, 6000);
    
    int timePassed_1ms = 0;
    int rdm_sleep_time_ms = 7000;//Any value greater than 6000 to force the if condition inside the while loop to excute the very first time
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        ++timePassed_1ms;
        if(timePassed_1ms >= rdm_sleep_time_ms)
        {
            timePassed_1ms = 0;
            //generate a random integer number between 4, and 6 seconds
            rdm_sleep_time_ms = uniform_dist(e1);

            this->_currentPhase = this->_currentPhase == TrafficLightPhase::red? TrafficLightPhase::green : TrafficLightPhase::red;
            TrafficLightPhase messagePhase = this->_currentPhase;
            _messageQueue.send(std::move(messagePhase));
        }
    }
}

