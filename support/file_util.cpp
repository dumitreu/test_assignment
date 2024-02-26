#include "commondefs.hpp"
#include "file_util.hpp"
#include "str_util.hpp"
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
#if !defined(_XOPEN_SOURCE) && _XOPEN_SOURCE < 500
#define _XOPEN_SOURCE 500
#endif
#include <ftw.h>
#include <unistd.h>
#include <pwd.h>
#elif defined(PLATFORM_WINDOWS)
#include <WinSock2.h>
#include <Shlobj.h>
#include <Shlobj_core.h>
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Shell32.lib")
#elif defined(PLATFORM_APPLE)
#include <unistd.h>
#include <removefile.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <fstream>

#ifdef USE_FILE_MAGIC
#include <magic.h>
#include "lzari.hpp"
#endif


namespace lins {

    namespace file_util {

        EXCEPTION_CLASS(invalid_magic_library, "unable to initialize magic library")
        EXCEPTION_CLASS(invalid_magic_database, "cannot load magic database")

        std::string get_temp_dir() {
        #ifdef __linux
            return "/tmp";
        #endif
            return std::string();
        }

        uint64_t dir_entry::file_size() const {
            return lins::file_util::file_size(path_);
        }

        bool dir_entry::is_file() const {
            return file_exists(path_);
        }

        bool dir_entry::is_dir() const {
            return dir_exists(path_);
        }

        lins::timespec_wrapper dir_entry::modify_time() const {
#if defined(PLATFORM_WINDOWS)
            struct _stat attrib;
            if(_stat(path_.c_str(), &attrib) == 0) {
                return attrib.st_mtime;
            } else {
                return -1LL;
            }
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
            struct stat attrib;
            if (stat(path_.c_str(), &attrib) == 0) {
                return attrib.st_mtim;
            } else {
                return -1LL;
            }
#elif defined(PLATFORM_APPLE)
            struct stat attrib;
            if (stat(path_.c_str(), &attrib) == 0) {
                return attrib.st_mtimespec;
            } else {
                return -1LL;
            }
#endif
        }

        lins::timespec_wrapper dir_entry::access_time() const {
#if defined(PLATFORM_WINDOWS)
            struct _stat attrib;
            if (_stat(path_.c_str(), &attrib) == 0) {
                return attrib.st_atime;
            } else {
                return -1LL;
            }
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
            struct stat attrib;
            if (stat(path_.c_str(), &attrib) == 0) {
                return attrib.st_atim;
            } else {
                return -1LL;
            }
#elif defined(PLATFORM_APPLE)
            struct stat attrib;
            if (stat(path_.c_str(), &attrib) == 0) {
                return attrib.st_atimespec;
            } else {
                return -1LL;
            }
#endif
        }

        lins::timespec_wrapper dir_entry::create_time() const {
#if defined(PLATFORM_WINDOWS)
            struct _stat attrib;
            if (_stat(path_.c_str(), &attrib) == 0) {
                return attrib.st_ctime;
            } else {
                return -1LL;
            }
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
            struct stat attrib;
            if (stat(path_.c_str(), &attrib) == 0) {
                return attrib.st_ctim;
            } else {
                return -1LL;
            }
#elif defined(PLATFORM_APPLE)
            struct stat attrib;
            if (stat(path_.c_str(), &attrib) == 0) {
                return attrib.st_ctimespec;
            } else {
                return -1LL;
            }
#endif
        }

        std::string home_dir(bool end_with_path_sep) {
            std::string final_result{};

        #if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
            struct passwd pwd;
            struct passwd *result;
            size_t bufsize = 16384;
            std::unique_ptr<char[]> buf(new char[bufsize]);

            if(getpwuid_r(getuid(), &pwd, buf.get(), bufsize, &result) == 0 &&
               result &&
               result->pw_dir)
            {
                final_result = std::string(result->pw_dir);
            }
        #elif defined(PLATFORM_WINDOWS)
            CHAR buff[2048];
            GetEnvironmentVariableA("USERPRFILE", buff, sizeof(buff));
            final_result = buff;
        #endif

            if(final_result.empty()) {
                throw std::runtime_error{"user home dir name is empty string"};
            }
            if(end_with_path_sep && final_result[final_result.size() - 1] != lins::file_util::native_path_separator<std::string>::val()[0]) {
                final_result += lins::file_util::native_path_separator<std::string>::val();
            }

            return final_result;
        }


        std::string config_dir() {
        #if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
            std::string home = home_dir();
            return  home.empty() ? std::string() : home + "/.config";
        #elif defined(PLATFORM_WINDOWS)
            TCHAR szPath[MAX_PATH];
            if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath))) {
                return lins::str_util::to_utf8(szPath);
            }
            return std::string{};
        #endif
            return {};
        }

        std::string cache_dir() {
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
            std::string home = home_dir();
            return  home.empty() ? std::string() : home + "/.cache";
#elif defined(PLATFORM_WINDOWS)
            TCHAR szPath[MAX_PATH];
            if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath))) {
                return lins::str_util::to_utf8(szPath);
            }
            return std::string{};
#endif
            return {};
        }



        std::string shared_library_ext() {
        #if defined(PLATFORM_WINDOWS)
            return "dll";
        #else
            return "so";
        #endif
        }

        std::string current_executable_dir() {
            std::string module_dir;

        #if defined(PLATFORM_WINDOWS)
            std::vector<char> path;
            path.resize(MAX_PATH + 1);
            if(GetModuleFileNameA(0, &path[0], MAX_PATH)) {
                std::string module_name = (CHAR *)&path[0];
                size_t slash_pos = module_name.find_last_of("\\");
                if(slash_pos != std::string::npos) {
                    module_dir = module_name.substr(0, slash_pos);
                }
            }
        #else
        #endif

            return module_dir;
        }

        std::string current_executable_path() {
            std::string module_path;
        #if defined(PLATFORM_WINDOWS)
            std::vector<char> path;
            path.resize(MAX_PATH + 1);
            if(GetModuleFileNameA(0, &path[0], MAX_PATH)) {
                module_path = (char *) &path[0];
            }
        #else
        #endif
            return module_path;
        }

        std::vector<std::string> read_text_file(const std::string &file_name) {
            std::vector<std::string> result;
            std::ifstream ifs;
            ifs.open(file_name.c_str());
            if(ifs.good()) {
                std::string buf;
                while(std::getline(ifs, buf)) {
                    result.push_back(buf);
                }
            }
            return result;
        }

        std::string current_dir() {
        #ifdef __linux
            std::unique_ptr<char[]> cwd(new char[64 * 1024]);
            if(getcwd(cwd.get(), sizeof(cwd))) {
                return std::string(cwd.get());
            }
        #endif
            return std::string();

        }

        EXCEPTION_CLASS(directory_opening_error, "failed to open directory")
        EXCEPTION_CLASS(file_loading_error, "failed to load file")

        std::vector<std::string> get_dir_list(const std::string &dir) {
            std::vector<std::string> files;
        #if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
            DIR *dp;
            struct dirent *dirp;
            if((dp  = opendir(dir.c_str())) == 0) {
                throw directory_opening_error();
            }
            while ((dirp = readdir(dp))) {
                if(dirp->d_type == DT_REG) {
                    files.push_back(std::string(dirp->d_name));
                }
            }
            closedir(dp);
        #endif
            return files;
        }

        std::string get_random_file_name(int len) {
            std::string res;
            static char chars[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','1','2','3','4','5','6','7','8','9','0'/*,'-','_','%','^','&','#','@','$'*/};
            res.reserve();
            for(int i = 0; i < len; i++) {
                res += chars[rand() % (sizeof(chars) / sizeof(chars[0]))];
            }
            return res;
        }

        std::string extract_file_ext(const std::string &file_name) {
            std::vector<std::string> tokens = lins::str_util::str_tok<std::string>(file_name, ".");
            if(tokens.size() < 2) {
                return "";
            }
            return tokens[tokens.size() - 1];
        }

        std::string extract_file_name(const std::string &path) {
            std::vector<std::string> tokens = lins::str_util::str_tok<std::string>(path, native_path_separator<std::string>::val(), true);
            while(tokens.size() && tokens[tokens.size() - 1].empty()) {
                auto end_it{tokens.end()}; --end_it;
                tokens.erase(end_it);
            }
            if(tokens.size() >= 1) {
                return tokens[tokens.size() - 1];
            } else {
                return std::string{};
            }
        }

        std::string extract_file_dir(const std::string &path) {
            std::vector<std::string> tokens = lins::str_util::str_tok<std::string>(path, native_path_separator<std::string>::val(), true);
            while(tokens.size() && tokens[tokens.size() - 1].empty()) {
                auto end_it{tokens.end()}; --end_it;
                tokens.erase(end_it);
            }
            if(tokens.size() > 1) {
                std::string out;
                for(int i = 0; i < (int)tokens.size() - 1; ++i) {
                    if(i == 0 || tokens[(std::size_t)i].size()) {
                        out += tokens[(std::size_t)i] + native_path_separator<std::string>::val();
                    }
                }
                return out;
            } else {
                return std::string{};
            }
        }

#ifdef USE_FILE_MAGIC
        namespace detail {

            template<typename T> struct magic_deleter;
            template<> struct magic_deleter<::magic_set> { void operator()(magic_set *p) const { ::magic_close(p); } };
            template<class T> using magic_ptr = std::unique_ptr<T, magic_deleter<T>>;

            class mgc_detector {
                class mgc_data {
                    mgc_data() {}
                public:
                    static mgc_data &inst() { static mgc_data ins{}; return ins; }
                    std::vector<std::uint8_t> const &data() const { return data_; }
                private:
                    std::vector<std::uint8_t> data_{lzari{}.decode(__magic_mgc, __magic_mgc_len)};
                };
            public:
                mgc_detector() {
                    void *buf_buf[1]{(void *)mgc_data::inst().data().data()};
                    size_t size_buf[1]{(size_t)mgc_data::inst().data().size()};
                    ::magic_load_buffers(magic_handle_.get(), buf_buf, size_buf, 1);
                }
                std::string file(const std::string &pat_str) const {
                    char const *res{::magic_file(magic_handle_.get(), pat_str.c_str())}; if(res) { return res; } return {};
                }
                std::string buffer(void const *data, std::size_t data_size) const {
                    char const *res{::magic_buffer(magic_handle_.get(), data, data_size)}; if(res) { return res; } else { return {}; }
                }

            private:
                magic_ptr<magic_set> magic_handle_{::magic_open(/*MAGIC_ERROR | */MAGIC_MIME)};
            };
        }

        std::string file_type(const std::string &pat_str) {
            if(file_exists(pat_str)) { return detail::mgc_detector{}.file(pat_str); }
            return {};
        }

        std::string data_type(void const *data, std::size_t data_size) {
            if(data && data_size) { return detail::mgc_detector{}.buffer(data, data_size); }
            return {};
        }

        std::string data_type(std::vector<std::uint8_t> const &data) {
            return data_type(data.data(), data.size());
        }

        std::string data_type(std::string const &data) {
            return data_type(data.data(), data.size());
        }
#endif

        bool dir_exists(const std::string &file_name) {
        #if defined(PLATFORM_LINUX) || defined(PLATFORM_UNIXISH) || defined(PLATFORM_APPLE) || defined(PLATFORM_ANDROID)
            struct stat sb;

            if(stat(file_name.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
                return true;
            }
            return false;
        #elif defined(PLATFORM_WINDOWS)
            DWORD dwAttrib = GetFileAttributesA(file_name.c_str());

            return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
                (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
        #endif
        }

        bool file_exists(const std::string &file_name) {
        #if defined(PLATFORM_LINUX) || defined(PLATFORM_APPLE) || defined(PLATFORM_ANDROID)
            struct stat sb;

            if(stat(file_name.c_str(), &sb) == 0 && S_ISREG(sb.st_mode)) {
                return true;
            }
            return false;
        #elif defined(PLATFORM_WINDOWS)
            DWORD dwAttrib = GetFileAttributesA(file_name.c_str());
            return (dwAttrib != INVALID_FILE_ATTRIBUTES && ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == 0));
        #endif
        }

        std::string real_path(const std::string &p) {
        #if defined(PLATFORM_LINUX) || defined(PLATFORM_APPLE) || defined(PLATFORM_ANDROID)
            std::string res;
            if(p.size()) {
                std::vector<char> resolved(p.size() * 2);
                res = realpath(p.c_str(), &resolved[0]);
            }
            return res;
        #elif defined(PLATFORM_WINDOWS)
            std::string res;
            if (p.size()) {
                std::vector<char> resolved(p.size() * 2 + 1);
                res = _fullpath(&resolved[0], p.c_str(), p.size() * 2);
            }
            return res;
        #endif
        }

        EXCEPTION_CLASS(stat_calling_error, "stat() calling error")
        EXCEPTION_CLASS(file_size_getting_error, "file size examining error")

        uint64_t file_size(const std::string &fn) {
        #if defined(PLATFORM_LINUX) || defined(PLATFORM_APPLE) || defined(PLATFORM_ANDROID)
            struct stat statbuf;
            if (stat(fn.c_str(), &statbuf) == -1) {
                throw stat_calling_error();
            }
            return static_cast<uint64_t>(statbuf.st_size);
        #elif defined(PLATFORM_WINDOWS)
            HANDLE hFile = CreateFileA(fn.c_str(), GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, 0);
            if(hFile == INVALID_HANDLE_VALUE) {
                throw file_size_getting_error();
            }
            LARGE_INTEGER size;
            if (!GetFileSizeEx(hFile, &size)) {
                CloseHandle(hFile);
                throw file_size_getting_error();
            }
            CloseHandle(hFile);
            return size.QuadPart;
        #endif
        }

        bool move_file(const std::string &from, const std::string &to) {
            return rename(from.c_str(), to.c_str()) == 0;
        }

        bool copy_file(std::string const &from, std::string const &to) {
            if(file_exists(from)) {
                file_ptr source{fopen(from.c_str(), "rb")};
                if(file_exists(to)) {
                    file_ptr dest{fopen(to.c_str(), "wb")};
                    size_t size;
                    char buf[BUFSIZ];
                    while((size = fread(buf, 1, BUFSIZ, source.get()))) { fwrite(buf, 1, size, dest.get()); }
                    return true;
                } else if(dir_exists(to)) {
                    file_ptr dest{fopen((to + native_path_separator<std::string>::val() + extract_file_name(from)).c_str(), "wb")};
                    size_t size;
                    char buf[BUFSIZ];
                    while((size = fread(buf, 1, BUFSIZ, source.get()))) { fwrite(buf, 1, size, dest.get()); }
                    return true;
                } else if(dir_exists(extract_file_dir(to))) {
                    file_ptr dest{fopen(to.c_str(), "wb")};
                    size_t size;
                    char buf[BUFSIZ];
                    while((size = fread(buf, 1, BUFSIZ, source.get()))) { fwrite(buf, 1, size, dest.get()); }
                    return true;
                }
            }
            return false;
        }

        bool mk_dir(const std::string &dir) {
        #if defined(PLATFORM_LINUX) || defined(PLATFORM_APPLE) || defined(PLATFORM_ANDROID)
            char tmp[256];
            char * p = nullptr;
            size_t len;

            snprintf(tmp, sizeof(tmp),"%s", dir.c_str());
            len = strlen(tmp);
            if(tmp[len - 1] == native_path_separator<std::string>::val()[0]) {
                tmp[len - 1] = 0;
            }
            for(p = tmp + 1; *p; p++) {
                if(*p == native_path_separator<std::string>::val()[0]) {
                    *p = 0;
                    ::mkdir(tmp, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
                    *p = native_path_separator<std::string>::val()[0];
                }
            }
            ::mkdir(tmp, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
            return true;
        #elif defined(PLATFORM_WINDOWS)
            return CreateDirectoryA(dir.c_str(), 0);
        #endif
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
        static int do_remove(const char *fpath, const struct stat64 */*sb*/, int tflag, struct FTW */*ftwbuf*/) {
            if(tflag == FTW_F) {
                ::unlink(fpath);
            } else if(tflag == FTW_DP) {
                ::rmdir(fpath);
            }
            return 0;
        }
#endif
        bool rm_dir(const std::string &path) {
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
            if (::nftw64(path.c_str(), do_remove, 100, FTW_DEPTH | FTW_PHYS) == -1) {
                return false;
            }
            return true;
#elif defined(PLATFORM_WINDOWS)
            return RemoveDirectoryA(path.c_str());
#elif defined(__APPLE__)
            removefile_state_t s;
            s = removefile_state_alloc();
            return removefile(path.c_str(), s, REMOVEFILE_RECURSIVE | REMOVEFILE_KEEP_PARENT) == 0;
#endif
        }
        //////////////////////////////////////////////////////////////////////////////////////////////////////

        bool match_wildcard(const std::string &pat_str, const std::string &str_str) {
            const char *pat = pat_str.c_str();
            const char *str = str_str.c_str();
            while(*str) {
                switch(*pat) {
                    case '?':
                        if(*str == '.') return false;
                        break;
                    case '*':
                        return !*(pat + 1) ||
                        match_wildcard(pat + 1, str) ||
                        match_wildcard(pat, str + 1);
                    default:
                        if(*str != *pat) return false;
                        break;
                }
                ++str;
                ++pat;
            }
            while(*pat == '*') ++pat;
            return !*pat;
        }

        bool delete_fs_entry(const std::string &fn) {
            if(dir_exists(fn)) {
                return rm_dir(fn);
            } else if(file_exists(fn)) {
        #if defined(PLATFORM_LINUX) || defined(PLATFORM_APPLE) || defined(PLATFORM_ANDROID)
                return ::unlink(fn.c_str()) == 0;
        #elif defined(PLATFORM_WINDOWS)
                return DeleteFileA(fn.c_str());
        #endif
            }
            return false;
        }

        bool is_empty(std::string const &dir) {
            bool contains{false};
            for_dir_tree(dir, [&](dir_entry const &) { contains = true; return false; }, false);
            return !contains;
        }

        std::vector<std::uint8_t> load_from_file(const std::string &fn, std::int64_t how_much) {
            std::deque<std::uint8_t> result{};
            if(!file_exists(fn)) {
                throw file_loading_error{};
            }
            std::ifstream file{};
            file.open(fn.c_str(), std::ios_base::in | std::ios_base::binary);
            if(file.is_open()) {
                int c{};
                std::int64_t total_read{0LL};
                while((c = file.get()) != -1) {
                    result.push_back(static_cast<std::uint8_t>(c));
                    ++total_read;
                    if(how_much > 0 && total_read >= how_much) {
                        break;
                    }
                }
                file.close();
            }
            return std::vector<std::uint8_t>{result.begin(), result.end()};
        }

        std::string load_text_file(const std::string &fn) {
            std::vector<char> result;
            if(!file_exists(fn)) {
                return std::string{};
            }
            std::ifstream file;
            file.open(fn.c_str(), std::ios_base::in | std::ios_base::binary);
            if(file.is_open()) {
                std::vector<char> buf(16384);
                std::streamsize some{0};
                while((some = file.readsome(&buf[0], buf.size()))) {
                    result.insert(result.end(), buf.begin(), buf.begin() + some);
                }
            }
            return {result.begin(), result.end()};
        }

        bool save_to_file(const std::string &fn, const std::vector<std::uint8_t> &data) {
            std::ofstream ofs;
            ofs.open(fn.c_str(), std::ios_base::out | std::ios_base::binary);
            if(ofs.is_open()) {
                ofs.write(reinterpret_cast<const char *>(data.data()), static_cast<ssize_t>(data.size()));
                ofs.close();
                return true;
            } else {
                return false;
            }
        }

        bool save_to_file(const std::string &fn, const std::string &data) {
            std::ofstream ofs;
            ofs.open(fn.c_str(), std::ios_base::out | std::ios_base::binary);
            if(ofs.is_open()) {
                ofs.write(reinterpret_cast<const char *>(data.data()), static_cast<ssize_t>(data.size()));
                ofs.close();
                return true;
            } else {
                return false;
            }
        }

        bool save_to_file(const std::string &fn, const void *data, ptrdiff_t size) {
            std::ofstream ofs;
            ofs.open(fn.c_str(), std::ios_base::out | std::ios_base::binary);
            if(ofs.is_open()) {
                if(data && size) {
                    ofs.write(reinterpret_cast<const char *>(data), size);
                }
                ofs.close();
                return true;
            } else {
                return false;
            }
        }

        bool is_absolute_path(const std::string &fn) {
            if(fn.size() == 0) {
                return false;
            }
            if(fn[0] == native_path_separator<std::string>::val()[0]) {
                return true;
            }
            return false;
        }

        std::string suffix_file_name_number(std::string const &fn, int num) {
            std::string tfn{fn};
            std::string sn{lins::str_util::itoa(static_cast<std::int64_t>(num))};
            auto dot_pos{tfn.find('.')};
            if(dot_pos != std::string::npos) {
                return tfn.substr(0, dot_pos) + "(" + sn + ")" + tfn.substr(dot_pos);
            } else {
                return tfn + "(" + sn + ")";
            }
        }

    }
}
