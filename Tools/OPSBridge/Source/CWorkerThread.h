/**
*
* Copyright (C) 2010-2012 Saab Dynamics AB
*   author Lennart Andersson <nnnn@saabgroup.com>
*
* Copyright (C) 2018-2024 Lennart Andersson.
*
* This file is part of OPS (Open Publish Subscribe).
*
* OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
*/

/*
	A small and simple framework for simplifying [worker]threads.
*/

#pragma once

#include <string>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <thread>
#include <vector>

// Forward declaration
class CWorkerThread;

#define UNUSED(expr) (void)(expr);

// ------------------------------------------------------------------------
// Base class for work to be done by a CWorkerThread
//
// Implement the Run() method. 
//
// For lengthy executions you should periodically call the terminated() 
// method to check if you are requested to exit the Run() method. 
//
class CWorkItem
{
	friend class CWorkerThread;
public:
	CWorkItem() : m_notified(false), m_terminateRequested(false)
	{
	}

	virtual ~CWorkItem() 
	{
	}

	virtual void Terminate()
	{
		m_terminateRequested = true;
		Notify();
	}

protected:
	// Variables used for notification handling	
	std::mutex m_mtx;
	std::condition_variable m_cv;
	bool m_notified;

	void Notify()
	{
		{
			std::lock_guard<std::mutex> lck(m_mtx);
			m_notified = true;
		}
		m_cv.notify_one();
	}

	void WaitForNotify()
	{
		std::unique_lock<std::mutex> lck(m_mtx);
		m_cv.wait(lck, [this] { return m_notified; });
		m_notified = false;
	}

	bool WaitForNotifyWithTimeout(int64_t const millis)
	{
		std::unique_lock<std::mutex> lck(m_mtx);
		bool notified = m_cv.wait_for(
			lck,
			std::chrono::milliseconds(millis),
			[this] { return m_notified; });
		m_notified = false;
		return notified;
	}

	// Variables used for terminate handling	
	volatile bool m_terminateRequested;

	// Returns true if terminatation has been requested
	inline bool terminated()
	{
		return m_terminateRequested;
	}

	// 
	virtual void Run() = 0;
};

// ------------------------------------------------------------------------
// Thrown from CWorkerThreadManager.Add() if a CWorkItemEx() is added
//
struct Invalid_Usage : public std::exception
{
	const char* what() const noexcept { return "Invalid usage"; }
};

// ------------------------------------------------------------------------
// Derived work item class that creates an internal CWorkerThread to do 
// the work when the Start() method is called.
//
// NOTE!!!	CWorkItemEx and its derived classes MUST NOT be added to the
//			CWorkerThreadManager !!!
//
class CWorkItemEx : public CWorkItem
{
public:
	CWorkItemEx();
	virtual ~CWorkItemEx();

	void Start();
	virtual void Stop();
	void WaitFor();
	void TerminateAndWaitFor();

private:
	CWorkerThread* m_WorkerThread;
};

// ------------------------------------------------------------------------
// Abstract class for user implementation of error logging etc.
// 
// An instance of a derived class can be given to the CWorkThreadManager to
// be used if exceptions are caught by the CWorkerThread outside of the
// given CWorkItem's Run() method.
//
class CWorkerErrorLogger 
{
public:
	virtual void onError(const std::string errorString, unsigned int errorId) = 0;
    virtual void onInformation(const std::string infoString, unsigned int infoNumber) { UNUSED(infoString); UNUSED(infoNumber); };
};

// ------------------------------------------------------------------------
// Class uses a Thread to perform the given CWorkItem.
//
// When created it starts a thread and calls the Run() method of the given
// CWorkItem. When the Run() method returns the thread is terminated.
//
class CWorkerThread 
{
	friend class CWorkItemEx;
    friend class CWorkerThreadManager;
public:
	CWorkerThread(CWorkItem* const workItem, bool const ownsWorkItem = true);
	virtual ~CWorkerThread();
protected:
	CWorkItem* m_WorkItem;

	void WaitFor();

    // Check if the thread has exited
    bool IsFinished();
private:
	bool m_ownsWorkItem;

	void run();

	std::atomic_bool m_running;
	std::thread m_thread;
};

// ------------------------------------------------------------------------
// Singleton class that may be used to keep track of started 
// CWorkerThread's / CWorkItem's
//
class CWorkerThreadManager
{
	friend class CWorkerThread;
private:
    bool m_shuttingDown;
	// These should not be accessible
	CWorkerThreadManager();
	CWorkerThreadManager(const CWorkerThreadManager &) = delete;
	CWorkerThreadManager & operator=(const CWorkerThreadManager &) = delete;
	~CWorkerThreadManager();
public:
	// Access method to the singleton instance
	static CWorkerThreadManager* Instance();

	// Set CWorkerErrorLogger to use for error logging
	// The error logger will be called from different threads
	void SetErrorLogger(CWorkerErrorLogger* logger);

	// Adds the given WorkerThread to the list of WorkerThread's 
	// CWorkerThreadManager takes over the ownership of the worker thread
	void Add(CWorkerThread *worker);

	// Creates a WorkerThread with the given WorkItem
	void Add(CWorkItem* workItem, bool ownsWorkItem = true);

    // Cleanup threads that have exited
    // Will delete WorkerThread's that have finished (including owned WorkItem's)
    void CleanUp();

	// Tries to stop all known WorkerThread's and delete them
	void StopAll();
protected:
	static CWorkerThreadManager* _instance;
	std::vector<CWorkerThread*> m_vWorkers;
	CWorkerErrorLogger* m_ErrorLogger;
};
