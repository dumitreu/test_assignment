#pragma once

#include "../../commondefs.hpp"
#include "../scoped_locker.hpp"
#include "../rw_lock.hpp"
#include "../../timespec_wrapper.hpp"
#include <WinSock2.h>

namespace lins {

    class mutex {
    public:
	    mutex(bool initially_owned = false, wchar_t *name = NULL);
        ~mutex();
        bool lock(timespec_wrapper = eternity, bool write = true);
        bool unlock(bool write = true);
        void *platform_handle();

    private:
        mutex(const mutex&);
        mutex &operator=(const mutex&);

    private:
        HANDLE mutex_;
    };


    class semaphore {
    public:
        semaphore(uint32_t initial_count = 1);
        ~semaphore();
        bool lock(timespec_wrapper = eternity, bool = true);
        bool unlock(bool = true);
        void *platform_handle();

    private:
        semaphore(const semaphore&);
        semaphore &operator=(const semaphore&);

    private:
        HANDLE sem_;
    };


    class event {
    public:
        event(bool autoreset = true, bool initial_set = false);
        ~event();
        bool set();
        void reset();
        bool wait(timespec_wrapper msec = eternity);
        bool lock(timespec_wrapper = eternity, bool = true);
        bool unlock(bool = true);
        void *platform_handle();

    private:
        event(const event&);
        event &operator=(const event&);

    private:
        HANDLE evnt_;
    };

/*
    class rw_lock {
    public:
	    rw_lock(void);
	    virtual ~rw_lock(void);

	    bool rdlock(timespec_wrapper = eternity);
	    bool wrlock(timespec_wrapper = eternity);
	    bool rdunlock();
	    bool wrunlock();
	    bool lock(timespec_wrapper = eternity, bool write = true);
	    bool unlock(bool write = true);
	    void *platform_handle() {
		    return 0;
	    }

    private:
	    void LockReader();
	    void UnlockReader();
	    void LockWriter();
	    void UnlockWriter();

	    rw_lock(const rw_lock &cReadWriteLock);
	    const rw_lock& operator=(const rw_lock &cReadWriteLock);
	    void IncrementReaderCount();
	    void DecrementReaderCount();
	    HANDLE m_hWriterEvent;
	    HANDLE m_hNoReadersEvent;
	    int m_nReaderCount;

	    CRITICAL_SECTION m_csLockWriter;
	    CRITICAL_SECTION m_csReaderCount;
    };
*/
/*
    class spin_lock {
    public:
        spin_lock() {
        }

        bool lock(timespec_wrapper = eternity, bool write = true);
        bool unlock(bool write = true);
        void *platform_handle();

    private:
        spin_lock(const spin_lock&);
        spin_lock &operator=(const spin_lock&);

    private:
	    long lfsSpinLock;
    };
*/
    class thread_base {
    public:
        enum priority {
            IDLE = -15,
            LOWEST = -2,
            BELOW_NORMAL = -1,
            NORMAL = 0,
            ABOVE_NORMAL = 1,
            HIGHEST =  2,
            TIME_CRITICAL = 15,
            UNKNOWN
        };

    public:
        thread_base(bool joinable = true);
        virtual ~thread_base();
        virtual void terminate();
        virtual bool termination() const;
        bool create();
        bool suspend();
        bool resume();
        bool running() const;
        bool wait(timespec_wrapper = eternity);
        bool kill();
        void *os_handle();
        priority thread_priority();
        bool set_priority(priority);
        static void sleep(timespec_wrapper = eternity);
        static void yield();
        static int system_cpus_installed();

    protected:
        virtual void entry() = 0;
	    virtual bool perform_per_thread_init();

    private:
        thread_base(const thread_base &);
        thread_base &operator=(const thread_base &);

    private:
        bool joinable_;
        bool termination_requested_;
        mutex thread_obj_mutex_;
        HANDLE thread_obj_;
        event wait_cond_;

    private:
	    static DWORD WINAPI thread_func(LPVOID);
        //static void *thread_func(void *thr_data);
    };

}
