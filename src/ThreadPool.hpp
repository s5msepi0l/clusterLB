#ifndef LEAN_AND_MEAN_THREADS
#define LEAN_AND_MEAN_THREADS

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>

#include "networking.hpp"

//template<typename T>
void pop_front(std::vector<struct n_params> &v) {
	if (v.size() > 0) {
		v.erase(v.begin());
	}
}

class ThreadPool {
public:
	std::vector<struct n_params> n_queue; // passive task queue 
	
	ThreadPool(int thread_count) : pool(thread_count) {		
		this->max_threads = thread_count-1;
		this->active_threads = 0;
		this->running = true;
		
		for (int i = 0; i < this->pool.size(); i++) {
			this->pool[i] = std::thread([](){});
		}

		this->master = std::thread(&ThreadPool::n_master_task, this);

		std::cout << "Constructor activated\n";
	}

	unsigned int execute(struct n_params param) //return task queue storlek
	{
		this->n_queue.push_back(param);

		return this->n_queue.size();
	}

	~ThreadPool() {
		if (this->pool.size() > 0) {
			for (int i = 0; i<this->pool.size(); i++){
				if (pool[i].joinable()) {
				this->pool[i].join();
				}
			}
		}
		this->running = false;
		if (this->master.joinable()) {
			this->master.join();
		}
		std::cout << "Destructor activated\n";
	}

private:
		int max_threads;
		int active_threads;

		std::vector<std::thread> pool; //Slave

		std::mutex mutexPool;
		std::thread master; //Master, ger ut arbete åt threads på samma gang man gör annat skit
		bool running;

		void n_master_task()
		{
			while (this->running) {
				//this->mutexPool.lock();
				//this->mutexPool.unlock();
				
				if (this->n_queue.size() > 0) {
					std::cout << "side";
				for (int i = 0; i< this->pool.size(); i++) {
					if (this->pool[i].joinable()) {
						this->pool[i].join();
						if (this->pool.size() < this->max_threads && this->n_queue.size() > 0){
							this->pool[i] = std::thread(handle_request, n_queue[0]);
							pop_front(n_queue);
							
							}
						}
					}

				}
				std::this_thread::sleep_for(std::chrono::milliseconds(200)); //sleep så he it slösar cpu cycles
			}
		}
};

#endif