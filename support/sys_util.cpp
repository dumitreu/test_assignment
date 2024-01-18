#include "commondefs.hpp"
#include "sys_util.hpp"
#include "file_util.hpp"
#include "str_util.hpp"
#if defined(PLATFORM_LINUX)
#include <mntent.h>
#include <libgen.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#ifdef USE_BACK_TRACE
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <cxxabi.h>
#endif
#elif defined(PLATFORM_WINDOWS)
#include <WinSock2.h>
#include <wincrypt.h>
#endif

namespace lins {

    namespace sys_util {

#if defined(PLATFORM_LINUX)
        std::string backtrace() {
#ifdef USE_BACK_TRACE
            unw_cursor_t cursor;
            unw_context_t context;

            unw_getcontext(&context);
            unw_init_local(&cursor, &context);

            std::stringstream ss;

            while(unw_step(&cursor) > 0) {
                unw_word_t offset, pc;
                unw_get_reg(&cursor, UNW_REG_IP, &pc);
                if(pc == 0) {
                    break;
                }
                ss << "0x" << lins::str_util::utoa(pc, 16);

                char sym[256];
                if(unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
                    std::string sym_str{sym};
                    int status;
                    char *demangled = abi::__cxa_demangle(sym, nullptr, nullptr, &status);
                    if (status == 0) {
                        sym_str = demangled;
                    }
                    ss << ' ' << sym_str << "+0x" << lins::str_util::utoa(offset, 16) << '\n';
                } else {
                    ss << " -- error: unable to obtain symbol name for this frame\n";
                }
            }

            return ss.str();
#else
            return std::string{};
#endif
        }
#else
        std::string backtrace() {
            return {};
        }
#endif

        int is_root() {
#if defined(PLATFORM_LINUX) || defined(PLATFORM_UNIXISH) || defined(PLATFORM_POSIX) || defined(PLATFORM_ANDROID)
            return getuid() == 0;
#else
            // TODO:
            return false;
#endif
        }

        static uint64_t multiplier(const std::string &s) {
            if((s == "kB") || (s == "KB") || (s == "Kb") || (s == "kb") || (s == "K") || (s == "k")) return 1024;
            if((s == "MB") || (s == "mB") || (s == "Mb") || (s == "mb") || (s == "M") || (s == "m")) return 1024 * 1024;
            if((s == "GB") || (s == "gB") || (s == "Gb") || (s == "gb") || (s == "G") || (s == "g")) return 1024 * 1024 * 1024;
            if((s == "B") || (s == "b") || s.empty()) return 1;
            return 1;
        }

        void mem_info(uint64_t &total_mem, uint64_t &free_mem) {
#if defined(PLATFORM_LINUX)
            using std::ios_base;

            total_mem = 0ll;
            free_mem = 0ll;

            std::ifstream stat_stream("/proc/meminfo", std::ios_base::in);

            std::string mname, memunits;
            uint64_t memsize;

            stat_stream >> mname >> memsize >> memunits;
            total_mem = memsize * multiplier(memunits);

            stat_stream >> mname >> memsize >> memunits;
            free_mem = memsize * multiplier(memunits);
            stat_stream.close();
#else
            total_mem = 0ULL;
            free_mem = 0ULL;
#endif
        }

        void mem_usage(uint64_t &vm_usage, uint64_t &resident_set) {
#if defined(PLATFORM_LINUX)
            using std::ios_base;
            using std::string;

            vm_usage     = 0ll;
            resident_set = 0ll;

            std::ifstream stat_stream("/proc/self/stat", std::ios_base::in);

            std::string pid, comm, state, ppid, pgrp, session, tty_nr;
            std::string tpgid, flags, minflt, cminflt, majflt, cmajflt;
            std::string utime, stime, cutime, cstime, priority, nice;
            std::string O, itrealvalue, starttime;

            uint64_t vsize;
            long rss;

            stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
                        >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
                        >> utime >> stime >> cutime >> cstime >> priority >> nice
                        >> O >> itrealvalue >> starttime >> vsize >> rss;

            stat_stream.close();

            long page_size = sysconf(_SC_PAGE_SIZE);
            vm_usage     = vsize;
            resident_set = rss * page_size;
#elif defined(PLATFORM_WINDOWS)
            MEMORYSTATUSEX status;
            status.dwLength = sizeof(status);
            GlobalMemoryStatusEx(&status);
            resident_set = status.ullTotalPhys;
            vm_usage = status.ullTotalPhys - status.ullAvailPhys;
#else
            vm_usage     = 0ll;
            resident_set = 0ll;
#endif
        }

        std::string get_exec_path() {
#if defined(PLATFORM_LINUX)
            std::size_t dest_len = 1024;
            char path[1024];
            path[0] = 0;
            if(readlink("/proc/self/exe", path, dest_len) != -1) {
                return lins::file_util::extract_file_dir(path);
            }
            return path;
#elif defined(PLATFORM_WINDOWS)
            TCHAR szFileName[MAX_PATH];
            GetModuleFileName(0, szFileName, MAX_PATH);
#ifdef _TCHAR_DEFINED
            return lins::str_util::to_utf8(szFileName);
#else
            return szFileName;
#endif
#else
            return {};
#endif
        }

        std::string host_name() {
            std::string result{};
#ifdef PLATFORM_LINUX
            struct addrinfo hints, *info, *p;
            int gai_result{0};

            char hostname[1024];
            hostname[1023] = 0;
            gethostname(hostname, 1023);

            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_CANONNAME;

            if((gai_result = getaddrinfo(hostname, "http", &hints, &info)) != 0) {
                return {};
            }

            for(p = info; p; p = p->ai_next) {
                result = p->ai_canonname;
                break;
            }
            freeaddrinfo(info);
#endif
            return result;
        }


        std::string system_name() {
#if defined(_WIN64)
            return "win64";
#elif defined(_WIN32)
            return "win32";
#elif defined(PLATFORM_APPLE)
    #if TARGET_OS_IPHONE
            return "iphone";
    #elif TARGET_IPHONE_SIMULATOR
            return "simulator";
    #elif TARGET_OS_MAC
            return "macos";
    #else
            return "apple";
    #endif
#elif defined(PLATFORM_LINUX)
            struct utsname buf;
            uname(&buf);
            std::string result(buf.sysname);
            result += std::string(" ") + buf.release;
            result += std::string(" ") + buf.version;
            return result;
#elif defined(PLATFORM_UNIXISH)
            return "unix";
#elif defined(PLATFORM_POSIX)
            return "posix";
#else
            return "unknown";
#endif
        }

        std::string system_vendor() {
            std::string res{};
#if defined(PLATFORM_LINUX)
            auto vendor_info{lins::file_util::load_from_file("/sys/devices/virtual/dmi/id/sys_vendor")};
            res = lins::str_util::trim(std::string{vendor_info.begin(), vendor_info.end()});
#endif
            return res;
        }

        std::string system_uuid() {
            std::string res{};
#if defined(PLATFORM_LINUX)
            bool found{false};
            struct mntent *ent;
            FILE *mounts_file;
            mounts_file = setmntent("/proc/mounts", "r");
            if(mounts_file) {
                std::string root_fs{};
                while(nullptr != (ent = getmntent(mounts_file))) {
                    if(std::string{ent->mnt_dir} == "/") {
                        root_fs = lins::file_util::extract_file_name(ent->mnt_fsname);
                        found = true;
                        break;
                    }
                }
                endmntent(mounts_file);
                if(found) {
                    for(
                        auto it{std::filesystem::directory_iterator("/dev/disk/by-uuid")};
                        it != std::filesystem::directory_iterator();
                        ++it
                        ) {
                        if(std::filesystem::is_symlink(it->symlink_status())) {
                            std::string fname{lins::file_util::extract_file_name(it->path())};
                            std::string slnk{lins::file_util::extract_file_name(std::filesystem::read_symlink(*it))};
                            if(slnk == root_fs) {
                                res = fname;
                                break;
                            }
                        }
                    }
                }
            }
#endif
            return res;
        }

        ssize_t gen_cs_rand(void *buff, std::size_t buff_size) {
            int result{0};
            if(buff && buff_size) {
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
                result = ::getrandom(buff, buff_size, GRND_RANDOM);
#elif defined(PLATFORM_APPLE)
                arc4random_buf(buff, buff_size);
                result = buff_size;
#elif defined(PLATFORM_WINDOWS)
                HCRYPTPROV hCryptProv{0};
                if(CryptAcquireContextW(&hCryptProv, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
                    if(CryptGenRandom(hCryptProv, buff_size, (unsigned char *)buff)) {
                        result = buff_size;
                    }
                }
#endif
            }
            return result;
        }

    }

}
