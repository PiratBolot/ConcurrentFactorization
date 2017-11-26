#ifndef BUSY_THREAD_FACTORIZATOR_H
#define BUSY_THREAD_FACTORIZATOR_H

#include <fstream>
#include <sstream>
#include "BusyThread.h"
#include "Factorizator.h"
#include "ThreadPool.h"

class ConcurrentFactorizator : public BusyThread {
public:
	enum class State {
		Valid = 0, Invalid = 1
	};

	ConcurrentFactorizator() = delete;
	
	ConcurrentFactorizator(int argc, char** argv, size_t threadNumber) : pool(threadNumber), state(State::Valid),
		inputFilename(argv[1]), outputFileName(argv[2]) {
		input.open(argv[1], std::ios_base::in);
		if (!input) {			
			state = State::Invalid;
		}
		output.open(argv[2], std::ios_base::trunc);
		if (!output) {			
			state = State::Invalid;
		}
	}

	bool isCorrect() const {
		return state == State::Valid;
	}

	void task() {
		std::string line;
		uint64_t number;

		while (!input.eof()) {
			input >> line;
			std::istringstream converter(line);
			converter >> number;
			if (!converter.eof()) {
				std::cerr << "Incorrect input line: " << line << std::endl;
				continue;
			}
			SyncPoint();
			pool.enqueue([this, number]() -> void {
				Factorizator f(number);
				f.factorization();
				std::unique_lock<std::mutex> lock(write_mutex);
				suspend_cond.wait(lock, [this]() -> bool {
					return !suspended;
				});
				output << f.toString() << std::endl;
				//std::cout << output.fail() << std::endl;
			});
		}
		pool.joinAll();
		input.close();
		output.close();
		std::cout << "Operation is completed" << std::endl;
	}

	void suspend() {
		{
			std::lock_guard<std::mutex> lock(write_mutex);
			suspended = true;
			BusyThread::suspend();
			pool.suspend();
		}
		output.close();
		output.clear();
	}

	void resume() {	
		output.open(outputFileName.c_str(), std::ios_base::app);
		suspended = false;
		suspend_cond.notify_all();
		BusyThread::resume();
		pool.resume();
	}

	void abort() {	
		BusyThread::abort();
		suspended = false;
		suspend_cond.notify_all();
		pool.abort();
		input.close();
		output.close();		
	}

	bool isDone() const {
		return BusyThread::isDone();
	}

private:	 
	std::atomic_bool suspended = false;
	std::condition_variable suspend_cond;
	std::mutex write_mutex;
	std::string inputFilename;
	std::string outputFileName;
	ThreadPool pool;
	std::ifstream input;
	std::ofstream output;
	State state;
};

#endif // !BUSY_THREAD_FACTORIZATOR_H