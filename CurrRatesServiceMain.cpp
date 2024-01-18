#include <WinSock2.h>
#include <tchar.h>

#include <iostream>
#include <list>
#include <vector>
#include <string>

#include "src/globals.hpp"
#include "src/configuration.hpp"
#include "src/exchange_rates_fetch_logic.hpp"
#include "src/croncpp.h"
#include "src/WinReg.hpp"

static SERVICE_STATUS        g_ServiceStatus = {0};
static SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
static HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

static VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
static VOID SvcInstall();
static VOID UninstallSvc();
static VOID StartSvc();
static VOID StopSvc();
static VOID WINAPI ServiceCtrlHandler(DWORD);

#define SERVICE_NAME  _T("Currency Exchange Rates Downloading Service")

static void print_install_usage(int argc, TCHAR* argv[]) {
    DEFINE_ARG_TSTRINGS_VECTOR(args, argc, argv);

    std::cout << "usage:" << std::endl;
#ifdef UNICODE
    std::cout << lins::str_util::to_utf8(args[0]);
#else
    std::cout << args[0];
#endif
    std::cout << " install -c <drive:\\path\\to\\config_file.cfg>"
        << " -d <drive:\\path\\to\\data_file.db>"
        << " -l <drive:\\path\\to\\log_file.log>"
        << " -f <drive:\\path\\to\\failed_file.db>"
        << std::endl;
    std::cout << "(The configuration file will be created if not exists but directory path must exist. Also directories for the rest of files must exist and be writable)" << std::endl;
}

bool set_conf_file_name(std::string const &conf_file_name) {
    bool res{false};
    try {
        std::wstring testSubKey = L"SYSTEM\\CurrentControlSet\\Services\\";
#ifdef UNICODE
        testSubKey += SERVICE_NAME;
#else
        testSubKey += lins::str_util::from_utf8(SERVICE_NAME);
#endif
        winreg::RegKey key{HKEY_LOCAL_MACHINE, testSubKey};
        key.SetStringValue(L"ConfigPath", lins::str_util::from_utf8(conf_file_name));
    } catch (const std::exception&) {
    }
    return res;
}

std::optional<std::string> get_conf_file_name() {
    std::optional<std::string> res{};
    try {
        std::wstring testSubKey = L"SYSTEM\\CurrentControlSet\\Services\\";
#ifdef UNICODE
        testSubKey += SERVICE_NAME;
#else
        testSubKey += lins::str_util::from_utf8(SERVICE_NAME);
#endif
        winreg::RegKey key{HKEY_LOCAL_MACHINE, testSubKey};
        res = lins::str_util::to_utf8(key.GetStringValue(L"ConfigPath"));
    } catch (std::exception const &e) {
        LOG << "error querying registry value: " << e.what() << std::endl;
    }
    return res;
}
 
bool setup_log(bool enabled, std::string const &log_file_name) {
    static std::unique_ptr<std::ofstream> log_file{};

    bool res{false};
    if(enabled) {
        if(good_file_name_or_candidate(log_file_name)) {
            log_file = std::make_unique<std::ofstream>();
            log_file->open(log_file_name);
            logging::set_log(log_file.get());
            res = true;
        } else {
            logging::set_log_null();
        }
    } else {
        logging::set_log_null();
    }
    return res;
}

int _tmain (int argc, TCHAR *argv[]) {
    DEFINE_ARG_TSTRINGS_VECTOR(args, argc, argv);

    bool conf_file_name_given{ false };
    std::string conf_file_name{};
    bool db_file_name_given{ false };
    std::string db_file_name{};
    bool log_file_name_given{ false };
    std::string log_file_name{};
    bool fail_file_name_given{ false };
    std::string fail_file_name{};

    if (args.size() > 1) {
        if (args[1] == _T("install")) {
            global_config = std::make_unique<lins::thread_safe_wrapper_stl<configuration>>();

            for (int i{ 2 }; i < args.size(); ++i) {
                if (
                    !conf_file_name_given &&
                    (args[i] == _T("-c") || args[i] == _T("-C") || args[i] == _T("/c") || args[i] == _T("/C"))
                    &&
                    (int)args.size() > i + 1
                ) {
                    ++i;
#ifdef UNICODE
                    conf_file_name = lins::str_util::to_utf8(args[i]);
#else
                    conf_file_name = args[i];
#endif
                    if (good_file_name_or_candidate(conf_file_name)) {
                        conf_file_name_given = true;
                    } else {
                        conf_file_name_given = false;
                        conf_file_name.clear();
                    }
                    continue;
                }
                if (
                    !db_file_name_given &&
                    (args[i] == _T("-d") || args[i] == _T("-D") || args[i] == _T("/d") || args[i] == _T("/D"))
                    &&
                    (int)args.size() > i + 1
                ) {
                    ++i;
#ifdef UNICODE
                    db_file_name = lins::str_util::to_utf8(args[i]);
#else
                    db_file_name = args[i];
#endif
                    if (good_file_name_or_candidate(db_file_name)) {
                        db_file_name_given = true;
                    } else {
                        db_file_name_given = false;
                        db_file_name.clear();
                    }
                    continue;
                }
                if (
                    !log_file_name_given &&
                    (args[i] == _T("-l") || args[i] == _T("-L") || args[i] == _T("/l") || args[i] == _T("/L"))
                    &&
                    (int)args.size() > i + 1
                ) {
                    ++i;
#ifdef UNICODE
                    log_file_name = lins::str_util::to_utf8(args[i]);
#else
                    log_file_name = args[i];
#endif
                    if (good_file_name_or_candidate(log_file_name)) {
                        log_file_name_given = true;
                    } else {
                        log_file_name_given = false;
                        log_file_name.clear();
                    }
                    continue;
                }
                if (
                    !fail_file_name_given &&
                    (args[i] == _T("-f") || args[i] == _T("-F") || args[i] == _T("/f") || args[i] == _T("/F"))
                    &&
                    (int)args.size() > i + 1
                ) {
                    ++i;
#ifdef UNICODE
                    fail_file_name = lins::str_util::to_utf8(args[i]);
#else
                    fail_file_name = args[i];
#endif
                    if (good_file_name_or_candidate(fail_file_name)) {
                        fail_file_name_given = true;
                    } else {
                        fail_file_name_given = false;
                        fail_file_name.clear();
                    }
                    continue;
                }
            }
            if (!conf_file_name_given) {
                std::cout << "invalid config file path" << std::endl;
                print_install_usage(argc, argv);
                return 1;
            }
            if (!db_file_name_given) {
                std::cout << "invalid data file path" << std::endl;
                print_install_usage(argc, argv);
                return 1;
            }
            //if (!log_file_name_given) {
            //    std::cout << "invalid log file path" << std::endl;
            //    print_install_usage(argc, argv);
            //    return 1;
            //}
            if (!fail_file_name_given) {
                std::cout << "invalid failures file path" << std::endl;
                print_install_usage(argc, argv);
                return 1;
            }

            if (!config()--->load(conf_file_name, true)) {
                return 2;
            }

            SvcInstall();
            set_conf_file_name(conf_file_name);
            config()--->set_data_file_name(db_file_name);
            if (log_file_name_given) {
                config()--->set_enable_log_file(true);
                config()--->set_log_file_name(log_file_name);
            }
            config()--->set_sched_file_name(fail_file_name);
            config()--->store();
            return 0;
        } else if (args[1] == _T("uninstall")) {
            UninstallSvc();
            return 0;
        } else if (args[1] == _T("start")) {
            StartSvc();
            return 0;
        } else if (args[1] == _T("stop")) {
            StopSvc();
            return 0;
        }
    }

    OutputDebugString(_T("CurrRatesService: Main: Entry"));

    SERVICE_TABLE_ENTRY ServiceTable[]{
        {SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain},
        {NULL, NULL}
    };

    if (StartServiceCtrlDispatcher (ServiceTable) == FALSE) {
       OutputDebugString(_T("CurrRatesService: Main: StartServiceCtrlDispatcher returned error"));
       return GetLastError ();
    }

    OutputDebugString(_T("CurrRatesService: Main: Exit"));
    return 0;
}

VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv) {
    DEFINE_ARG_TSTRINGS_VECTOR(args, argc, argv);
    do {
        OutputDebugString(_T("CurrRatesService: ServiceMain: Entry"));

        g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

        if (g_StatusHandle == NULL) {
            OutputDebugString(_T("CurrRatesService: ServiceMain: RegisterServiceCtrlHandler returned error"));
            break;
        }

        // Tell the service controller we are starting
        ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
        g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwServiceSpecificExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 0;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) {
            OutputDebugString(_T("CurrRatesService: ServiceMain: SetServiceStatus returned error"));
        }

        /*
         * Perform tasks neccesary to start the service here
         */
        OutputDebugString(_T("CurrRatesService: ServiceMain: Performing Service Start Operations"));

        // Create stop event to wait on later.
        g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (g_ServiceStopEvent == NULL) {
            OutputDebugString(_T("CurrRatesService: ServiceMain: CreateEvent(g_ServiceStopEvent) returned error"));

            g_ServiceStatus.dwControlsAccepted = 0;
            g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
            g_ServiceStatus.dwWin32ExitCode = GetLastError();
            g_ServiceStatus.dwCheckPoint = 1;

            if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) {
                OutputDebugString(_T("CurrRatesService: ServiceMain: SetServiceStatus returned error"));
            }
            break;
        }

        // Tell the service controller we are started
        g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
        g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 0;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) {
            OutputDebugString(_T("CurrRatesService: ServiceMain: SetServiceStatus returned error"));
        }

        std::optional<std::string> conf_file_name{get_conf_file_name()};
        if (!conf_file_name) {
            break;
        }

        try {
            global_regular_queue = std::make_unique<lins::command_queue>(1, 10000);
            global_retry_queue = std::make_unique<lins::command_queue>(2, 10000);

            logging::init();
            logging::set_log_null();

            // configuration is a one of the key components of the application
            global_config = std::make_unique<lins::thread_safe_wrapper_stl<configuration>>();
            if (!config()--->load(*conf_file_name, true)) { 
                break;
            }
            // configuration renewal thread from persistence (file system) by "hardcoded" unix
            // cron-style schedule to enable users to change configuration parameters dynamically

            // setup logging from config
            bool log_enabled{ (*global_config)--->enable_log_file() };
            std::string log_file_name{ (*global_config)--->log_file_name() };
            setup_log(log_enabled, log_file_name);

            // configuration refreshing thread
            std::thread config_updater{[&]() {
                LOG << "starting thread config reloading thread" << std::endl;
                cron::cronexpr cfg_reloading_cron{cron::make_cron("*/5 * * * * *")}; // check every 5 seconds
                for (std::time_t next{cron::cron_next(cfg_reloading_cron, std::time(0))}; !global_termination;) {
                    std::time_t now{std::time(0)};
                    if (now > next) {
                        LOG << "reloading configuration" << std::endl;
                        next = cron::cron_next(cfg_reloading_cron, now);
                        if (config()--->reload()) {
                            // reconfigure logging accordibgly to config if needed
                            bool curr_log_enabled{(*global_config)--->enable_log_file()};
                            std::string curr_log_file_name{(*global_config)--->log_file_name()};
                            if (curr_log_enabled != log_enabled || curr_log_file_name != log_file_name) {
                                setup_log(curr_log_enabled, curr_log_file_name);
                                log_enabled = curr_log_enabled;
                                log_file_name = curr_log_file_name;
                            }
                        }
                    }
                    // for the reason of cron we cannot wait on some synchro object...
                    std::this_thread::sleep_for(std::chrono::milliseconds{200});
                }
            }};

            // an instance of the currency exchange rates downloader
            exchange_rates_fetch_logic erl{};
            erl.init();
            if (erl.ok()) {
                while (WaitForSingleObject(g_ServiceStopEvent, 500) != WAIT_OBJECT_0) {
                }
            }

            // stop globally and wait config renewal thread if running
            global_termination = true;
            if (config_updater.joinable()) {
                config_updater.join();
            }
        } catch (...) {
        }

        // cleanup
        OutputDebugString(_T("CurrRatesService: ServiceMain: Performing Cleanup Operations"));

        CloseHandle(g_ServiceStopEvent);

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 3;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) {
            OutputDebugString(_T("CurrRatesService: ServiceMain: SetServiceStatus returned error"));
        }
    } while(false);
    
    OutputDebugString(_T("CurrRatesService: ServiceMain: Exit"));
}


VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode) {
    OutputDebugString(_T("CurrRatesService: ServiceCtrlHandler: Entry"));

    switch (CtrlCode) {
        case SERVICE_CONTROL_STOP:
            OutputDebugString(_T("CurrRatesService: ServiceCtrlHandler: SERVICE_CONTROL_STOP Request"));

            if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING) {
                break;
            }

            // Perform tasks neccesary to stop the service here         
            g_ServiceStatus.dwControlsAccepted = 0;
            g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            g_ServiceStatus.dwWin32ExitCode = 0;
            g_ServiceStatus.dwCheckPoint = 4;

            if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE) {
                OutputDebugString(_T("CurrRatesService: ServiceCtrlHandler: SetServiceStatus returned error"));
		    }

            // This will signal the worker thread to start shutting down
            SetEvent (g_ServiceStopEvent);
            break;

        default:
            break;
    }

    OutputDebugString(_T("CurrRatesService: ServiceCtrlHandler: Exit"));
}

VOID SvcInstall() {
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    TCHAR szPath[MAX_PATH];

    if (!GetModuleFileName(NULL, szPath, MAX_PATH)) {
        printf("Cannot install service (%d)\n", GetLastError());
        return;
    }

    lins::str_util::tstring quoted_path = lins::str_util::tstring{_T("\"")} + szPath + lins::str_util::tstring{_T("\"")};

    // Get a handle to the SCM database. 

    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (NULL == schSCManager) {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Create the service

    schService = CreateService(
        schSCManager,              // SCM database 
        SERVICE_NAME,              // name of service 
        SERVICE_NAME,              // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_AUTO_START,        // start type 
        SERVICE_ERROR_NORMAL,      // error control type 
        quoted_path.c_str(),       // path to service's binary 
        NULL,                      // no load ordering group 
        NULL,                      // no tag identifier 
        NULL,                      // no dependencies 
        NULL,                      // LocalSystem account 
        NULL);                     // no password 

    if (schService == NULL)
    {
        printf("CreateService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }
    else printf("Service installed successfully\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

VOID UninstallSvc() {
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    SERVICE_STATUS ssStatus;

    // Get a handle to the SCM database. 

    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (NULL == schSCManager) {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Get a handle to the service.

    LPCTSTR szSvcName = SERVICE_NAME;

    schService = OpenService(
        schSCManager,       // SCM database 
        szSvcName,          // name of service 
        DELETE);            // need delete access 

    if (schService == NULL) {
        printf("OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    // Delete the service.

    if (!DeleteService(schService))
    {
        printf("DeleteService failed (%d)\n", GetLastError());
    }
    else printf("Service deleted successfully\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}


VOID StartSvc() {
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    SERVICE_STATUS ssStatus;

    // Get a handle to the SCM database. 

    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (NULL == schSCManager) {
        std::cout << "OpenSCManager failed: " << GetLastError() << std::endl;
        return;
    }

    // Get a handle to the service.

    LPCTSTR szSvcName = SERVICE_NAME;

    schService = OpenService(
        schSCManager,           // SCM database 
        szSvcName,              // name of service 
        SC_MANAGER_ALL_ACCESS); // need delete access 

    if (schService == NULL) {
        printf("OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    if (!StartService(schService, 0, nullptr)) {
        std::cout << "StartService failed: " << GetLastError() << std::endl;
    } else {
        std::cout << "Service started successfully\n" << std::endl;
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

VOID StopSvc() {
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    SERVICE_STATUS_PROCESS ssp;
    DWORD dwStartTime = GetTickCount();
    DWORD dwBytesNeeded;
    DWORD dwTimeout = 30000; // 30-second time-out
    DWORD dwWaitTime;

    // Get a handle to the SCM database. 

    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (NULL == schSCManager) {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Get a handle to the service.
    LPCTSTR szSvcName = SERVICE_NAME;

    schService = OpenService(
        schSCManager,         // SCM database 
        szSvcName,            // name of service 
        SC_MANAGER_ALL_ACCESS);

    if (schService == NULL) {
        printf("OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    // Make sure the service is not already stopped.

    if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
        printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
        goto stop_cleanup;
    }

    if (ssp.dwCurrentState == SERVICE_STOPPED) {
        printf("Service is already stopped.\n");
        goto stop_cleanup;
    }

    // If a stop is pending, wait for it.

    while (ssp.dwCurrentState == SERVICE_STOP_PENDING) {
        printf("Service stop pending...\n");

        // Do not wait longer than the wait hint. A good interval is 
        // one-tenth of the wait hint but not less than 1 second  
        // and not more than 10 seconds. 

        dwWaitTime = ssp.dwWaitHint / 10;

        if (dwWaitTime < 1000)
            dwWaitTime = 1000;
        else if (dwWaitTime > 10000)
            dwWaitTime = 10000;

        Sleep(dwWaitTime);

        if (!QueryServiceStatusEx(
            schService,
            SC_STATUS_PROCESS_INFO,
            (LPBYTE)&ssp,
            sizeof(SERVICE_STATUS_PROCESS),
            &dwBytesNeeded))
        {
            printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
            goto stop_cleanup;
        }

        if (ssp.dwCurrentState == SERVICE_STOPPED)
        {
            printf("Service stopped successfully.\n");
            goto stop_cleanup;
        }

        if (GetTickCount() - dwStartTime > dwTimeout)
        {
            printf("Service stop timed out.\n");
            goto stop_cleanup;
        }
    }

    // Send a stop code to the service.

    if (!ControlService(schService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssp)) {
        printf("ControlService failed (%d)\n", GetLastError());
        goto stop_cleanup;
    }

    // Wait for the service to stop.

    while (ssp.dwCurrentState != SERVICE_STOPPED) {
        Sleep(ssp.dwWaitHint);
        if (!QueryServiceStatusEx(
            schService,
            SC_STATUS_PROCESS_INFO,
            (LPBYTE)&ssp,
            sizeof(SERVICE_STATUS_PROCESS),
            &dwBytesNeeded))
        {
            printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
            goto stop_cleanup;
        }

        if (ssp.dwCurrentState == SERVICE_STOPPED)
            break;

        if (GetTickCount() - dwStartTime > dwTimeout)
        {
            printf("Wait timed out\n");
            goto stop_cleanup;
        }
    }
    printf("Service stopped successfully\n");

stop_cleanup:
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}
