#pragma once

#include "commondefs.hpp"
#include "file_util.hpp"
#include "str_util.hpp"

namespace lins {

namespace sys_util {

//extern const std::size_t Ki;
//extern const std::size_t Mi;
//extern const std::size_t Gi;

constexpr std::size_t Ki = 1024;
constexpr std::size_t Mi = Ki * Ki;
constexpr std::size_t Gi = Mi * Ki;
constexpr std::size_t Ti = Gi * Ki;

//constexpr bool little_endian();
//constexpr bool big_endian();
//constexpr bool pdp_endian();

#ifdef PLATFORM_WINDOWS
enum endian_t : uint32_t {
	LITTLE_ENDIAN = 0x00000001,
	BIG_ENDIAN = 0x01000000,
	PDP_ENDIAN = 0x00010000,
	UNKNOWN_ENDIAN = 0xFFFFFFFF
};

constexpr endian_t getEndianOrder() {
	return ((0xFFFFFFFF & 1) == LITTLE_ENDIAN) ? LITTLE_ENDIAN : ((0xFFFFFFFF & 1) == BIG_ENDIAN)
		? BIG_ENDIAN : ((0xFFFFFFFF & 1) == PDP_ENDIAN) ? PDP_ENDIAN : UNKNOWN_ENDIAN;
}
#endif


static constexpr bool little_endian() {
#ifdef PLATFORM_WINDOWS
    return getEndianOrder() == endian_t::LITTLE_ENDIAN;
#else
    return BYTE_ORDER == LITTLE_ENDIAN;
#endif
}

static constexpr bool big_endian() {
#ifdef PLATFORM_WINDOWS
    return getEndianOrder() == endian_t::BIG_ENDIAN;
#else
    return BYTE_ORDER == BIG_ENDIAN;
#endif
}

static constexpr bool pdp_endian() {
#ifdef PLATFORM_WINDOWS
    return getEndianOrder() == endian_t::PDP_ENDIAN;
#else
    return BYTE_ORDER == PDP_ENDIAN;
#endif
}

int is_root();

void mem_usage(uint64_t &vm_usage, uint64_t &resident_set);
void mem_info(uint64_t &total_mem, uint64_t &free_mem);
bool mem_trim(size_t = 0);
std::string get_exec_path();
std::string host_name();
std::string system_name();
std::string system_uuid();
std::string system_vendor();

enum {
    HOST_ORDER_LITTLE_ENDIAN = 0x03020100ul,
    HOST_ORDER_BIG_ENDIAN = 0x00010203ul,
    HOST_ORDER_PDP_ENDIAN = 0x01000302ul
};

static const union {
    unsigned char bytes[4];
    std::uint32_t value;
} host_order = { { 0, 1, 2, 3 } };

#define HOST_ORDER (lins::sys_util::host_order.value)
#define HOST_ORDER_LE (lins::sys_util::HOST_ORDER_LITTLE_ENDIAN)
#define HOST_ORDER_BE (lins::sys_util::HOST_ORDER_BIG_ENDIAN)
#define HOST_ORDER_PDP (lins::sys_util::HOST_ORDER_PDP_ENDIAN)

template<typename S_T>
static S_T check_dir_slash(const S_T &path) {
	if (path.size()) {
        if (path[path.size() - 1] != file_util::native_path_separator<S_T>()[0]) {
            return path + file_util::native_path_separator<S_T>();
		} else {
			return path;
		}
	}
    return file_util::native_path_separator<S_T>();
}

std::string backtrace();

ssize_t gen_cs_rand(void *, std::size_t);

#ifdef PLATFORM_WINDOWS
	template<typename FUNC_T>
	void for_reg_key(HKEY hk, 
					const lins::str_util::tstring &path, 
					FUNC_T apply, 
					REGSAM sam = KEY_ALL_ACCESS, 
					std::size_t max_nesting = -1, 
					bool values = true) 
	{
		HKEY key;
		LONG status = RegOpenKeyEx(hk, path.c_str(), 0, sam, &key);
		if(status == ERROR_SUCCESS) {
			tstring path_st = check_dir_slash(path);

			if(values) {
				std::vector<TCHAR> val_name(8192);
				DWORD val_name_size = val_name.size();
				DWORD val_type;
				std::vector<std::uint8_t> val_content(65536);
				DWORD val_content_size = val_content.size();
				for(DWORD i = 0;; i++) {
					LONG status;
					val_name_size = val_name.size();
					val_content_size = val_content.size();
					for(status = RegEnumValue(key, i, &val_name[0], &val_name_size, 0, &val_type, &val_content[0], &val_content_size); status != ERROR_SUCCESS; status = RegEnumValue(key, i, &val_name[0], &val_name_size, 0, &val_type, &val_content[0], &val_content_size)) {
						if(ERROR_NO_MORE_ITEMS == status) {
							break;
						}
						if(val_name_size > val_name.size()) {
							val_name.resize(val_name_size);
						}
						else {
							val_name_size = val_name.size();
						}
						if(ERROR_MORE_DATA == status) {
							if(val_content.size() < val_content_size) {
								val_content.resize(val_content_size);
							}
							else {
								val_content_size = val_content.size();
							}
						}
					}
					if(ERROR_NO_MORE_ITEMS == status) {
						break;
					}
					if(status == ERROR_SUCCESS) {
						apply(key, path_st + val_name.data(), val_name.data(), false, std::vector<std::uint8_t>(val_content.data(), val_content.data() + val_content_size), val_type);
					}
				}
			}

			if(max_nesting) {
				std::vector<tstring> subkeys;

				std::vector<TCHAR> subkey_name(255);
				DWORD index = 0;
				for(LONG status = RegEnumKey(key, index++, &subkey_name[0], subkey_name.size()); status == ERROR_SUCCESS; status = RegEnumKey(key, index++, &subkey_name[0], subkey_name.size())) {
					tstring apply_fn = path_st + subkey_name.data();
					for_reg_key(hk, apply_fn, apply, sam, max_nesting - 1, values);
					subkeys.push_back(subkey_name.data());
				}

				for(std::size_t i = 0; i < subkeys.size(); i++) {
					apply(key, path_st + subkeys[i].data(), subkeys[i].data(), true, std::vector<std::uint8_t>(), 0);
				}
			}

			RegCloseKey(key);
		}
	}
#endif

}

}
