#pragma once

#include "commondefs.hpp"
#include "timespec_wrapper.hpp"
#include <sys/types.h>
#ifdef PLATFORM_WINDOWS
#else
#include <dirent.h>
#endif
#include <errno.h>
#include <string>
#include <vector>
#include <iostream>
#include <istream>
#include <ostream>

namespace lins {

    namespace file_util {

        struct file_closer { void operator()(FILE *f) const { if(f) { ::fclose(f); } } };
        using file_ptr = std::unique_ptr<FILE, file_closer>;
    
        template<typename T>
        struct native_path_separator;

        template<>
        struct native_path_separator<std::string> {
            static std::string val() {
            #if defined(PLATFORM_WINDOWS)
                return "\\";
            #else
                return "/";
            #endif
            }
            static char sym() {
            #if defined(PLATFORM_WINDOWS)
                return '\\';
            #else
                return '/';
            #endif
            }
        };

        template<>
        struct native_path_separator<std::wstring> {
            static std::wstring val() {
            #if defined(PLATFORM_WINDOWS)
                return L"\\";
            #else
                return L"/";
            #endif
            }
            static wchar_t sym() {
            #if defined(PLATFORM_WINDOWS)
                return '\\';
            #else
                return '/';
            #endif
            }
        };

        std::string current_executable_dir();
        std::string home_dir(bool end_with_path_sep = false);
        std::string config_dir();
        std::string cache_dir();
        std::string current_executable_path();
        std::string shared_library_ext();
        std::vector<std::string> read_text_file(const std::string &);
        std::string current_dir();
        std::vector<std::string> get_dir_list(const std::string &);
        std::string get_random_file_name(int);
        std::string get_temp_dir();
        std::string extract_file_ext(const std::string &);
        std::string extract_file_name(const std::string &);
        std::string extract_file_dir(const std::string &);
        bool dir_exists(const std::string &);
        bool file_exists(const std::string &);
        uint64_t file_size(const std::string &);
        bool mk_dir(const std::string &);
        bool rm_dir(const std::string &);
        bool delete_fs_entry(const std::string &);
        bool is_empty(std::string const &dir);
        bool move_file(const std::string &from, const std::string &to);
        bool copy_file(const std::string &from, const std::string &to);
        std::vector<uint8_t> load_from_file(const std::string &, std::int64_t how_much = 0LL);
        std::string load_text_file(const std::string &fn);
        bool save_to_file(const std::string &, const std::vector<uint8_t> &);
        bool save_to_file(const std::string &fn, const void *data, ptrdiff_t size);
        bool save_to_file(const std::string &fn, const std::string &data);
        bool is_absolute_path(const std::string &fn);
        bool match_wildcard(const std::string &pat_str, const std::string &str_str);

#ifdef USE_FILE_MAGIC
        std::string file_type(const std::string &pat_str);
        std::string data_type(void const *data, std::size_t data_size);
        std::string data_type(std::vector<std::uint8_t> const &data);
        std::string data_type(std::string const &data);
        extern unsigned char __magic_mgc[];
        extern std::size_t __magic_mgc_len;
#endif

        std::string suffix_file_name_number(std::string const &fn, int num);
        
        class dir_entry {
        public:
            dir_entry(const std::string &n = std::string()): path_(n) {
            }
            dir_entry(const dir_entry &that): path_(that.path_) {
            }
            dir_entry &operator=(const dir_entry &that) {
                if(&that != this) {
                    path_ = that.path_;
                }
                return *this;
            }
            std::string file_name() const {
                return extract_file_name(path_);
            }
            std::string file_ext() const {
                return extract_file_ext(path_);
            }
            void name(const std::string &fp) {
                path_ = fp;
            }
            const std::string &name() const {
                return path_;
            }
            lins::timespec_wrapper create_time() const;
            lins::timespec_wrapper access_time() const;
            lins::timespec_wrapper modify_time() const;
            bool is_file() const;
            bool is_dir() const;
            uint64_t file_size() const;
        
        public:
            std::string path_;
        };

#ifdef PLATFORM_WINDOWS
        template<typename T>
        void for_dir_tree(const std::string &dir_path, T apply, bool recursive = true) {
            std::string local_dir_path = dir_path;
            if(local_dir_path.size() == 0) {
                local_dir_path = std::string(".") + native_path_separator<std::string>::val();
            }
            if(local_dir_path[local_dir_path.size() - 1] != native_path_separator<std::string>::val()[0]) {
                local_dir_path += native_path_separator<std::string>::val();
            }
            if(!dir_exists(local_dir_path)) {
                return;
            }
            struct dirent *dir_entry_p;
            //DIR *dir_p = 0;
            HANDLE dir_p{ INVALID_HANDLE_VALUE };
            WIN32_FIND_DATAA ffd;
            bool fr{true};
            for(
                dir_p = FindFirstFileA((local_dir_path + "*.*").c_str(), &ffd);
                dir_p != INVALID_HANDLE_VALUE && fr;
                fr = FindNextFileA(dir_p, &ffd)
            ) {
                std::string p = local_dir_path + ffd.cFileName;
                
                if (std::string{ ffd.cFileName } == "." || std::string{ ffd.cFileName } == "..") {
                    continue;
                }

                dir_entry aplly_entry;
                if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    p += native_path_separator<std::string>::val();
                    aplly_entry.name(p);
                    apply(aplly_entry);
                    if(recursive) {
                        for_dir_tree(p, apply, true);
                    }
                } else {
                    aplly_entry.name(p);
                    apply(aplly_entry);
                }
            }
            if(dir_p != INVALID_HANDLE_VALUE) {
                FindClose(dir_p);
            }
        }
#else
        template<typename T>
        void for_dir_tree(const std::string &dir_path, T apply, bool recursive = true) {
            std::string local_dir_path = dir_path;
            if(local_dir_path.size() == 0) {
                local_dir_path = std::string(".") + native_path_separator<std::string>::val();
            }
            if(local_dir_path[local_dir_path.size() - 1] != native_path_separator<std::string>::val()[0]) {
                local_dir_path += native_path_separator<std::string>::val();
            }
            if(!dir_exists(local_dir_path)) {
                return;
            }
            struct dirent *dir_entry_p;
            DIR *dir_p = 0;
            for(dir_p = opendir(local_dir_path.c_str()); dir_p && (dir_entry_p = readdir(dir_p)); ) {
                if(memcmp(dir_entry_p->d_name, ".", 1) == 0 || memcmp(dir_entry_p->d_name, "..", 2) == 0) {
                    continue;
                }
                std::string p = local_dir_path + dir_entry_p->d_name;
                dir_entry aplly_entry;
                if(dir_exists(p)) {
                    p += native_path_separator<std::string>::val();
                    aplly_entry.name(p);
                    if(!apply(aplly_entry)) {
                        break;
                    }
                    if(recursive) {
                        for_dir_tree(p, apply, true);
                    }
                } else {
                    aplly_entry.name(p);
                    if(!apply(aplly_entry)) {
                        break;
                    }
                }
            }
            if(dir_p) {
                closedir(dir_p);
            }
        }
#endif
    
    }
}
