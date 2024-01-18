#include "threading.hpp"

#ifdef _WIN64
    #include "threading/windows/mt.cpp"
#elif _WIN32
	#include "threading/windows/mt.cpp"
#elif __APPLE__
    #include "threading/pthreads/mt.cpp"
#elif __ANDROID__
    #include "threading/stl/mt.cpp"
#elif __linux
    #include "threading/pthreads/mt.cpp"
#elif __unix
    #include "threading/pthreads/mt.cpp"
#elif __posix
    #include "threading/pthreads/mt.cpp"
#endif
