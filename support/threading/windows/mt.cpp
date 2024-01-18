#include "../../commondefs.hpp"
#include "mt.hpp"
//#include "utils.h"
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

namespace lins {

	thread_base::thread_base(bool joinable) :
		joinable_(joinable),
		termination_requested_(false),
		thread_obj_(0),
		wait_cond_(false, true)
	{
	}

	thread_base::~thread_base() {
	}

	bool thread_base::perform_per_thread_init() {
		return true;
	}

	DWORD WINAPI thread_base::thread_func(LPVOID thr_data) {
		if(thr_data) {
			thread_base *this_ = reinterpret_cast<thread_base *>(thr_data);
			if(!this_->perform_per_thread_init()) {
				return -1;
			}
			if(this_->joinable_) {
				this_->entry();
				{
					scoped_locker<mutex> lck(this_->thread_obj_mutex_);
					this_->thread_obj_ = 0;
				}
				this_->wait_cond_.set();
			} else {
				this_->entry();
			}
		}
		return 0;
	}

	bool thread_base::create() {
		scoped_locker<mutex> lck(thread_obj_mutex_);
		if(thread_obj_ == 0) {
			wait_cond_.reset();
			thread_obj_ = ::CreateThread(0, 0, thread_func, reinterpret_cast<void *>(dynamic_cast<thread_base *>(this)), 0, 0);
			if (thread_obj_ != 0) {
				return true;
			} else {
				wait_cond_.set();
				return false;
			}
		} else {
			throw std::runtime_error("Multiple thread starting attempt for single thread object");
		}
		thread_obj_ = 0;
		return false;
	}

	bool thread_base::suspend() {
		::SuspendThread(thread_obj_);
		return false;
	}

	bool thread_base::resume() {
		::ResumeThread(thread_obj_);
		return false;
	}

	bool thread_base::running() const {
		DWORD result = WaitForSingleObject(thread_obj_, 0);
		if (result == WAIT_OBJECT_0) {
			return false;
		} else {
			return true;
		}

	}

	bool thread_base::wait(timespec_wrapper msec) {
		DWORD result = ::WaitForSingleObject(thread_obj_, msec.milliseconds());
		if (result == WAIT_OBJECT_0) {
			return true;
		} else {
			return false;
		}
	}

	void thread_base::sleep(timespec_wrapper msec) {
		::Sleep(msec.milliseconds());
	}

	void thread_base::yield() {
		::SwitchToThread();
	}

	bool thread_base::kill() {
		scoped_locker<mutex> lck(thread_obj_mutex_);
		if (::TerminateThread(thread_obj_, 0)) {
			thread_obj_ = 0;
			return true;
		}
		return false;
	}

	thread_base::priority thread_base::thread_priority() {
		int pri = ::GetThreadPriority(thread_obj_);
		if (THREAD_PRIORITY_ERROR_RETURN == pri) {
			throw std::runtime_error("error requesting thread priority");
		}
		return static_cast<thread_base::priority>(pri);
	}

	bool thread_base::set_priority(priority p) {
		return ::SetThreadPriority(thread_obj_, p) != 0;
	}

	void thread_base::terminate() {
		termination_requested_ = true;
	}

	bool thread_base::termination() const {
		return termination_requested_;
	}

	int thread_base::system_cpus_installed() {
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		return sysinfo.dwNumberOfProcessors;
	}

	void *thread_base::os_handle() {
		scoped_locker<mutex> lck(thread_obj_mutex_);
		return (void *)thread_obj_;
	}




	mutex::mutex(bool initially_owned, wchar_t *) {
		mutex_ = ::CreateMutex(0, initially_owned, 0);
		if (mutex_ == 0) {
			throw std::runtime_error("mutex initialization failed");
		}
	}

	mutex::~mutex() {
		if(!::CloseHandle(mutex_)) {
			throw std::runtime_error("mutex destroy failed");
		}
	}

	bool mutex::lock(timespec_wrapper msec, bool) {
		return WAIT_OBJECT_0 == ::WaitForSingleObject(mutex_, msec.milliseconds());
	}

	bool mutex::unlock(bool) {
		return ::ReleaseMutex(mutex_) != 0;
	}

	void *mutex::platform_handle() {
		return mutex_;
	}




	event::event(bool autoreset, bool initial_set) {
		evnt_ = ::CreateEvent(0, !autoreset, initial_set, 0);
		if (evnt_ == 0) {
			throw std::runtime_error("condition variable initialization failed");
		}
	}

	event::~event() {
		if(!::CloseHandle(evnt_)) {
			throw std::runtime_error("condition variable destroy failed");
		}
	}

	bool event::set() {
		return ::SetEvent(evnt_) != 0;
	}

	void event::reset() {
		::ResetEvent(evnt_);
	}

	bool event::wait(timespec_wrapper msec) {
		return ::WaitForSingleObject(evnt_, msec.milliseconds()) == WAIT_OBJECT_0;
	}

	bool event::lock(timespec_wrapper msec, bool) {
		return wait(msec);
	}

	bool event::unlock(bool) {
		return set();
	}

	void *event::platform_handle() {
		return evnt_;
	}




	semaphore::semaphore(uint32_t initial) {
		sem_ = ::CreateSemaphore(0, initial, MAXLONG, 0);
		if(!sem_) {
			throw std::runtime_error("semaphore initialization failed");
		}
	}

	semaphore::~semaphore() {
		if(!::CloseHandle(sem_)) {
			throw std::runtime_error("semaphore destroy failed");
		}
	}

	bool semaphore::lock(timespec_wrapper msec, bool) {
		return WAIT_OBJECT_0 == ::WaitForSingleObject(sem_, msec.milliseconds());
	}

	bool semaphore::unlock(bool /*write*/) {
		return ::ReleaseSemaphore(sem_, 1, 0) != 0;
	}

	void *semaphore::platform_handle() {
		return sem_;
	}



/*
	rw_lock::rw_lock(void): m_nReaderCount(0), m_hWriterEvent(NULL), m_hNoReadersEvent(NULL) {
		m_hWriterEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
		m_hNoReadersEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
		InitializeCriticalSection(&m_csLockWriter);
		InitializeCriticalSection(&m_csReaderCount);
	}

	rw_lock::~rw_lock(void) {
		DeleteCriticalSection(&m_csLockWriter);
		DeleteCriticalSection(&m_csReaderCount);
		CloseHandle(m_hWriterEvent);
		CloseHandle(m_hNoReadersEvent);
	}

	void rw_lock::LockReader() {
		bool bLoop = true;

		while (bLoop) {
			WaitForSingleObject(m_hWriterEvent, INFINITE);
			IncrementReaderCount();
			if (WaitForSingleObject(m_hWriterEvent, 0) != WAIT_OBJECT_0) {
				DecrementReaderCount();
			} else {
				bLoop = false;
			}
		}
	}

	void rw_lock::UnlockReader() {
		DecrementReaderCount();
	}

	void rw_lock::LockWriter() {
		EnterCriticalSection(&m_csLockWriter);
		WaitForSingleObject(m_hWriterEvent, INFINITE);
		ResetEvent(m_hWriterEvent);
		WaitForSingleObject(m_hNoReadersEvent, INFINITE);
		LeaveCriticalSection(&m_csLockWriter);
	}

	void rw_lock::UnlockWriter() {
		SetEvent(m_hWriterEvent);
	}

	void rw_lock::IncrementReaderCount() {
		EnterCriticalSection(&m_csReaderCount);
		m_nReaderCount++;
		ResetEvent(m_hNoReadersEvent);
		LeaveCriticalSection(&m_csReaderCount);
	}

	void rw_lock::DecrementReaderCount() {
		EnterCriticalSection(&m_csReaderCount);
		m_nReaderCount--;
		if (m_nReaderCount <= 0) {
			SetEvent(m_hNoReadersEvent);
		}
		LeaveCriticalSection(&m_csReaderCount);
	}

	bool rw_lock::rdlock(timespec_wrapper msec) {
		LockReader();
		return true;
	}

	bool rw_lock::wrlock(timespec_wrapper msec) {
		LockWriter();
		return true;
	}

	bool rw_lock::rdunlock() {
		UnlockReader();
		return true;
	}

	bool rw_lock::wrunlock() {
		UnlockWriter();
		return true;
	}

	bool rw_lock::lock(timespec_wrapper msec, bool write) {
		if(write) {
			return wrlock(msec);
		} else {
			return rdlock(msec);
		}
	}

	bool rw_lock::unlock(bool write) {
		if(write) {
			return wrunlock();
		} else {
			return rdunlock();
		}
	}
*/


/*
	bool spin_lock::lock(timespec_wrapper, bool) {
		while(InterlockedExchange(&lfsSpinLock, 1) == 1);
		return true;
	}

	bool spin_lock::unlock(bool) {
		InterlockedExchange(&lfsSpinLock, 0); 
		return true;
	}

	void *spin_lock::platform_handle() {
		return 0;
	}
*/

}
