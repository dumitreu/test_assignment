# test_assignment

##Application Deployment and Functioning

Copy this whole folder (together with content) into any desireable place on 
disk (which should be writeable) and execute install.bat and then start.bat.

This will install and start currency download service which, by default,
stores currency rates data inside CurrRatesService.db file (in JSON format).

During installation, configuration file is created and path to it is written
to the

"HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Currency Exchange Rates Downloading Service\ConfigPath"

registry value.

Here is the example of configuration file:

```
{
    "cron_string": "0 0 6 * * *",
    "data_file_name": "C:\\temp\\surr_rates_svc\\CurrRatesService.db",
    "enable_log_file": true,
    "failed_sched_cron_string": "0 */5 * * * *",
    "fetched_data_type": "json",
    "log_file_name": "C:\\temp\\surr_rates_svc\\CurrRatesService.log",
    "sched_file_name": "rates_fetch_sched.json"
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

You can edit config file while service is running and the service will
update values from config file... modifying config values you can vary
program behaviour.

Checking log file you can monitor current application activity.

Actual data is stored in json data file which contains two sections:

    - currencies dictionary
    - exchange rates grouped by date
    
inside of the top-level object.

Rates fetching is scheduled in configuration file

If service is failed to fetch rates data from remote, it schedules fetching
attempt into special file mentioned as "sched_file_name" in configuration which
is scanned by "failed_sched_cron_string" schedule in attempt to retry fetching
currency exchange rates data from server.

For now, the only format supported is JSON for everything data processing
and exchange. The benefit is that provided json implementation is hardware
accelerated so enabled to parse near 2 GB JSON text per second, so even many
years of data collecting will not be a problem in terms of performance.
