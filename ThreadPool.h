/**
 *
 * The thread pool class.
 * You can start any callable objects: functors, global and static function, and class methods
 * Update: also you can suspend, resume and abort running tasks
 */

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <atomic>
#include <utility>

class ThreadPool {
public:

	explicit ThreadPool(size_t threadsNumber);

	/**
	* Add a new task to the task queue: functors, functions, class methods
	*/
	template<class Fun, class... Args>
	auto enqueue(Fun&& f, Args&&... args)
		->std::future<typename std::result_of<Fun(Args...)>::type>;
	
	size_t getThreadNumber() const;

	/**
	* Join all worker threads
	*/
	void joinAll();

	void suspend();

	void resume();

	void abort();

	~ThreadPool();
private:
	size_t threadsNumber;

	// storage of worker threads
	std::vector<std::thread> workers;

	// the task queue
	std::queue<std::function<void()>> tasks;

	// synchronization
	std::atomic_bool aborted = false;
	std::atomic_bool suspended = false;
	std::atomic_bool stop = false;
	std::mutex queue_mutex;
	std::condition_variable condition;
	std::condition_variable resume_cond;
};

// Initialize a specified number of worker threads

inline ThreadPool::ThreadPool(size_t threadNumber) {

	// if the number of threads that system can handle concurrently more than {threadsNumber}, this value will be set
	if (threadNumber < std::thread::hardware_concurrency()) {
		this->threadsNumber = std::thread::hardware_concurrency();
	} else {
		this->threadsNumber = threadNumber;
	}
	for (size_t i = 0; i < this->threadsNumber; ++i)
		workers.emplace_back([this]() {
			for (;;) {
				std::function<void()> task;
				{
					std::unique_lock<std::mutex> lock(queue_mutex);
					condition.wait(lock, [this]() {
						return stop || !tasks.empty();
					});		
					resume_cond.wait(lock, [this]() {
						return !suspended;
					});
					if (aborted) {
						return;
					}
					if (stop && tasks.empty())
						return;
					task = std::move(tasks.front());
					tasks.pop();
				}
				task();
			}
		}
		);
}

/**
 * adds a task to the task queue and returns std::future on result
 */
template<class Fun, class ...Args>
inline auto ThreadPool::enqueue(Fun && f, Args && ...args)
-> std::future<typename std::result_of<Fun(Args ...)>::type>
{
	using return_type = typename std::result_of<Fun(Args...)>::type;

	auto task = std::make_shared<std::packaged_task<return_type()> >(
		std::bind(std::forward<Fun>(f), std::forward<Args>(args)...)
		);

	std::future<return_type> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		tasks.emplace([task]() { (*task)(); });
	}
	condition.notify_one();
	return res;
}

/** 
 * the destructor joins all threads
 */
inline ThreadPool::~ThreadPool() {
	joinAll();
}

inline size_t ThreadPool::getThreadNumber() const {
	return threadsNumber;
}

inline void ThreadPool::suspend() {
	std::unique_lock<std::mutex> lock(queue_mutex);
	suspended = true;
}

inline void ThreadPool::resume() {	
	std::unique_lock<std::mutex> lock(queue_mutex);
	suspended = false;
	resume_cond.notify_all();
}

inline void ThreadPool::abort() {
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		aborted = true;
		suspended = false;
	}
	resume_cond.notify_all();
	joinAll();
}

inline void ThreadPool::joinAll() {
	{		
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}
	condition.notify_all();
	for (auto& x : workers) {
		if (x.joinable()) {
			x.join();
		}
	}
}

#endif
