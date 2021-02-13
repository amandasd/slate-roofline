
#pragma once

#if defined(__cplusplus)
extern "C"
{
#endif

#if defined(USE_TIMEMORY)

    void set_papi_events(int nevents, const char** events);
    void initialize(int* argc, char*** argv);
    void finalize();
    void push_region(const char*);
    void pop_region(const char*);

#else

static inline void
set_papi_events(int, const char**)
{}

static inline void
initialize(int*, char***)
{}

static inline void
finalize()
{}

static inline void
push_region(const char*)
{}

static inline void
pop_region(const char*)
{}

#endif

#if defined(__cplusplus)
}
#endif
