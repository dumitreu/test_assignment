# Test Assignment

## Application Deployment
Application binaries and batch files to manage service installation/state are provided in  folder "dist". You can also find README.txt there.
In order to install a service you should copy the whole contents of the "dist" folder into any desireable place (folder) on your disk (for example, "D:\currency_fetcher" which should be available for write), change directory to that new location and, in command prompt of console execute install.bat and then start.bat. This will install, activate and start currency download service.

## Functioning
During installation, configuration file is created and path to it is written
to the

"HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Currency Exchange Rates Downloading Service\ConfigPath"

registry value.

Here is the example of configuration file (which is generated automatically during service installation process):

```
{
    "cron_string": "0 0 6 * * *",
    "data_file_name": "C:\\temp\\curr_rates_svc\\CurrRatesService.db",
    "enable_log_file": true,
    "failed_sched_cron_string": "0 */5 * * * *",
    "fetched_data_type": "json",
    "log_file_name": "C:\\temp\\curr_rates_svc\\CurrRatesService.log",
    "sched_file_name": "C:\\temp\\curr_rates_svc\\CurrRatesService.bad"
}
```

where

- "fetched_data_type": processing data format. for now "json" is supported only
- "cron_string": regular every day data fetch schedule (by default, 6:00 in the morning)
- "data_file_name": data file where all the data is stored
- "failed_sched_cron_string": scan retry schedule for failed fetches
- "sched_file_name": file containig dates for which rates fetching were failed
- "enable_log_file": switches logging
- "log_file_name": log file path

You can edit config file while service is running and the service will update values from config file. By modifying config values you can vary program behaviour. For example, you can turn logging on and off, change periodic events schedule.

Checking log file you can monitor current application activity.

Actual data is stored in json data file which contains two sections:

    - currencies dictionary (which is automatically maintained during service work)
    - exchange rates grouped by date (as the subobjects)
    
inside of the top-level object of the Database JSON document.

Rates fetching is scheduled in configuration file

If service is failed to fetch rates data from the remote, it schedules fetching attempts into the special separate file mentioned as "sched_file_name" in the configuration which is scanned by "failed_sched_cron_string" schedule attempting to retry fetching currency exchange rates data from server.

For now, the only format supported is JSON for data processing and exchange. Provided json implementation is hardware accelerated so enabled to parse near 2 GB of JSON text per second, so even many years of data collecting will not be a problem in the terms of performance.

## "support" folder
The "support" folder is my own code which is actually part used by this application, directly or via included headers

## Used 3rd party sources

 - OpenSSL library (TLS connection to the NBU server providing API)
 - SIMDJSON library (JSON parsing vectorized acceleration)
 - WinReg (C++ Wrapper Around Windows Registry C API)
