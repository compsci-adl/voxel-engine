#include <cstddef>

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#elif defined(__linux__)
#include <unistd.h>
#include <ios>
#include <fstream>
#elif defined(__APPLE__)
#include <mach/mach.h>
#endif

size_t getMemoryUsage() {
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS *)&pmc,
                         sizeof(pmc));
    return pmc.WorkingSetSize;
#elif defined(__linux__)
    long rss = 0L;
    std::ifstream statm("/proc/self/statm");
    if (statm.is_open()) {
        statm >> rss >> rss;
        statm.close();
    }
    return rss * sysconf(_SC_PAGESIZE);
#elif defined(__APPLE__)
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&t_info,
                  &t_info_count) != KERN_SUCCESS)
        return 0;
    return t_info.resident_size;
#else
    return 0; // Unsupported platform
#endif
}