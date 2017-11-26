#ifndef EXTENDED_THREAD_H
#define EXTENDED_THREAD_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <exception>

class BusyThread {
public:

	BusyThread() = default;	
	
	void start(bool isDetached = false) {
		thread = std::thread(&BusyThread::run, this);
		if (isDetached) {
			thread.detach();
		}
	}	

	void detach() {
		if (!thread.joinable()) {
			throw std::runtime_error("Thread was detached twice");
		}
		thread.detach();
	}

	bool joinable() const {
		return thread.joinable();
	}

	void join() {
		if (thread.joinable()) {
			thread.join();
		} else {
			std::unique_lock<std::mutex> ul(joinMutex);
			joinEvent.wait(ul, [=] {
				return done;
			});
		}
	}

	void suspend() {
		std::lock_guard<std::mutex> lock(stateMutex);
		state = State::sleep;
		stateEvent.notify_one();
	}

	void resume() {
		std::lock_guard<std::mutex> lock(stateMutex);
		state = State::wakeup;
		stateEvent.notify_one();
	}

	void abort() {
		setAbort(true);
	}

	static unsigned hardware_concurrency() {
		return std::thread::hardware_concurrency();
	}

	bool isDone() const {
		return done;
	}

private:

	enum class State {
		sleep = 0, wakeup = 1, abort = 3
	};

	class ThreadAbortException : std::exception {};

	void run() {
		try {
			task();
		} catch(std::runtime_error& err) {
			detachExceptionHandler(err);
		} catch (ThreadAbortException&) {
			aborted();
		} catch (std::exception& ex) {
			exceptionHandler(ex);
		} catch (...) {
			unknownExceptionHandler();
		}
		std::lock_guard<std::mutex> lock(joinMutex);
		done = true;
		joinEvent.notify_all();
		terminated();
	}

	std::thread thread;
	std::condition_variable stateEvent, joinEvent;
	std::mutex stateMutex, joinMutex;
	State state = State::wakeup;
	bool done = false;

protected:

	void SyncPoint(bool autoReset = false) {
		std::unique_lock<std::mutex> ul(stateMutex);
		stateEvent.wait(ul, [=] {
			auto result = ((int)state & (int)State::wakeup) > 0;
			if (state == State::abort)
				throw ThreadAbortException();
			if (autoReset)
				state = State::sleep;
			return result;
		});
	}

	void setAbort(bool set) {
		std::lock_guard<std::mutex> lock(stateMutex);
		if (set) {			
			state = State::abort;			
		} else {
			state = State::wakeup;
		}
		stateEvent.notify_one();
	} 

	virtual void task() = 0;
	virtual void aborted() {}
	virtual void detachExceptionHandler(std::runtime_error& exception) {}
	virtual void terminated() {}
	virtual void exceptionHandler(std::exception& exception) {}
	virtual void unknownExceptionHandler() {}

};

#endif // !EXTENDED_THREAD_H