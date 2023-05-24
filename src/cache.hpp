#include <iostream>
#include <sys/time.h>
#include <cstring>
#include <vector>
#include <chrono>

#include <thread>
#include <mutex>
#include <unistd.h>

#define uint_128 unsigned long long int
#define monitor_rate 300 //5min
#define expiration_rate_m 120

/*
caching is done via nodes that "expire" after a set amount of time,
monitored from a seperate thread running in the background
*/

class cache
{
public:
	cache(int bucket, uint_128 expiration) : bucket_size(bucket)
	{
		for (int i = 0; i<this->bucket_size; i++)
		{
			this->bucket[i] = {NULL, NULL, 0, 0, NULL};
		}

		this->monitor_cache = std::thread(cache::monitor_cache, this);
	}

	int insert(char *content, char *path)
	{
		int index = hash(path);

		for (int i = 0;; i++)
		{
			if (bucket[i].path == NULL)
			{
				//might be interpreted as the sizeof the first char, might need some refactoring
				struct cache_node buf = {content, path, sizeof(content), clock_t(), NULL};

				//buf memory might be corrupted because it is a rvalue
				std::memcpy(&bucket[index], (const void*)&buf, sizeof(buf));
				
				return 0;
			}
		}

		return 0;
	}


private:
	std::thread cache_monitor;
	bool thread_running;
	std::mutex mutex_p;

	void monitor_cache()
	{

		while (this->thread_running){	
			std::this_thread::sleep_for(std::chrono::milliseconds(monitor_rate * 1000));
		
			for (int i = 0; i<this->bucket.size(); i++)
			{
				//index control so it doesn't fuck everything up
				this->mutex_p.lock();
				if ((float(clock_t()) - float(this->bucket.at(i).set)) >= expiration_rate_m);
				{
					this->mutex_p.unlock();

					//clear local cache
					delete[] this->bucket[i].content;
				} else{
					this->mutex_p.unlock();
				}
				
			}
		}
	}

};

//incase of hashmap collision's it will basically just search for a empty cell (yes it's slow as fuck)
class cache_map {

public:
	cache_map(int bucket_sz) : bucket_sz(bucket_size)
	{
	}

	//this piece of shit need's jesus
	int insert_h(struct cache_node node, std::vector<struct cache_node> *lst)
	{
		int index = hash_h(node.path);

		if (lst[i].path != NULL) 
		{
			//travel through linked list until an empty
			//every iteration makes curr_node the head node low spacecomplexity
			struct cache_node *curr_node = &lst[i];
			for(int i = 0;;i++)
			{
				if (lst.next == NULL) //chain end reached
				{
					curr_node.cache.content = new char[sizeof(node.content)+1]; //+1 byte for null terminator
					break;
				}
				curr_node = currnode.cache_node; //move one chain up
			}
		}
	}

	~cache_map()
	{
		//implement me
		//should clear all present buffers out of memory
	}
private:
	std::vector<cache_node> bucket;
	int bucket_size;

	//pseudo random number generator
	unsigned int hash_h(char *key)
	{
		int len = strlen(key);
		int hash = 0;

		for (int i = 0; i<len; i++)
		{
			hash *= i;
			hash %=	bucket_size;
		}

		return hash;
	}

	//  linked list incase of hashmap collision  //
	struct cache_node{
		char *content;
		char *path;
		uint_128 size;
		clock_t set;

		struct cache_node *next;
	};
};
