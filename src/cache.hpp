#include <iostream>
#include <ctime>
#include <cstring>
#include <vector>
#include <chrono>

#include <thread>
#include <mutex>
#include <unistd.h>

#define uint_128 unsigned long long int
#define monitor_rate 10 //1 min
#define expiration_rate_m 300 //5 main

/*
caching is done via nodes that "expire" after a set amount of time,
monitored from a seperate thread running in the background
*/

struct Node {
	char *content;
	char *path;
	std::chrono::system_clock::time_point set;
};

class Cache
{
public:
	Cache(int bucket_sz) 
	: bucket_size(bucket_sz)
	{
		this->bucket.resize(bucket_sz);

		this->thread_running = true;
		this->cache_monitor = std::thread(&Cache::monitor_cache, this);
	}

	~Cache()
	{
		this->mutex_p.lock();
		this->thread_running = false;
		this->mutex_p.unlock();

		if (this->cache_monitor.joinable())
		{
			this->cache_monitor.join();
		}

		//clear cache
		for (int i = 0; i<this->bucket.size(); i++)
		{
			for (int j = 0; j<this->bucket[i].size(); j++)
			{
				delete[] this->bucket[i][j].content;
				delete[] this->bucket[i][j].path;
			}
		}

		this->bucket.clear();
		std::cout << "Destructor\n";
	}

	int insert_h(char *cache, char *path)
	{

		int index = hash(path);
		std::cout << index << "dwada\n";
		//sligtly faster than just iterating normaly as it doesn't have to increment and keep track of an int

		if (this->bucket[index].empty())
		{
			this->bucket[index].resize(1);
		}

		if (this->bucket[index][0].path == nullptr)
		{
			this->bucket[index][0].path = new char[strlen(path)+1];
			this->bucket[index][0].content = new char[strlen(cache)+1];
			
			strcpy(this->bucket[index][0].path, path);
			strcpy(this->bucket[index][0].content, cache);
				
			this->bucket[index][0].set = std::chrono::system_clock::now();
			return 0;
		}

		for (int j = 0;; j++)		
		{
			if (this->bucket[index][j].path == nullptr)
			{
				this->bucket[index][j].path = new char[strlen(path)+1];
				this->bucket[index][j].content = new char[strlen(cache)+1];
				strcpy(this->bucket[index][j].path, path);
				strcpy(this->bucket[index][j].content, cache);
				this->bucket[index][j].set = std::chrono::system_clock::now();
				
				return -1;
			}
		}
	}

	//nullptr if desired node is not found
	char *get(char *key)
	{
		int index = hash(key);
		
		for (int i = 0; i<this->bucket[index].size(); i++)
		{
			if (std::strcmp(this->bucket[index][i].path, key) == 0) //if pair is found return cache or if cell is empty return NULL
			{
				std::cout << "get\n";
				this->bucket[index][i].set = std::chrono::system_clock::now();
				return this->bucket[index][i].content;	
			}
		}

		return nullptr;
	}

private:
	std::vector<std::vector<struct Node>> bucket;
	int bucket_size;


	std::thread cache_monitor;
	std::mutex mutex_p;
	bool thread_running;

	unsigned int hash(char *key)
	{
		int len = strlen(key);
		int hash = 1;

		for (int i = 0; i<len; i++)
		{
			hash += i;
			hash *= hash;
		}
		return hash % this->bucket_size;
	}


	void monitor_cache()
	{
		while (this->thread_running)
		{
			this->mutex_p.unlock();

			std::this_thread::sleep_for(std::chrono::milliseconds(monitor_rate * 1000));
			for (int i = 0;i<this->bucket.size(); i++)
			{
				for (int j = 0; j<bucket[i].size(); j++)
				{
					//amount of time since the cache was last accesed
					if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - this->bucket[i][j].set).count() >=expiration_rate_m)
					{
						delete[] this->bucket[i][j].path;
						delete[] this->bucket[i][j].content;
						std::swap(this->bucket[i][j], this->bucket[i].back());
						this->bucket.pop_back();
					}
				}
			}
		}

		this->mutex_p.lock();
	}
};
