#include <node_api.h>

#define OK_OR_THROW(status, description)                \
    {                                                   \
        if (status != napi_ok)                          \
        {                                               \
            printf("napi_status -> %d\n", (int)status); \
            napi_throw_error(env, NULL, description);   \
        }                                               \
    }

#define THROW(description) napi_throw_error(env, NULL, description);

#ifdef DEBUG
#define DEBUG_INFO(...) printf(__VA_ARGS__)
#else
#define DEBUG_INFO(...)
#endif
