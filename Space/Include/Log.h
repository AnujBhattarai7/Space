#ifdef SP_BUILD_LOGS
// ---- PRINT ----
#define SP_CORE_PRINT(x)                      \
    {                                         \
        std::cout << "[CORE]: " << x << "\n"; \
    }
#define SP_APP_PRINT(x)                      \
    {                                        \
        std::cout << "[APP]: " << x << "\n"; \
    }

// ---- ERROR ----
#define SP_CORE_ERROR(x)                \
    {                                   \
        std::cout << "[ERROR][CORE]: "; \
        assert(false && x);             \
    }
#define SP_APP_ERROR(x)                \
    {                                   \
        std::cout << "[ERROR][APP]: "; \
        assert(false && x);             \
    }
#else
// ---- PRINT ----
#define SP_CORE_PRINT(x) \
    {                    \
    }
#define SP_APP_PRINT(x) \
    {                   \
    }

// ---- ERROR ----
#define SP_CORE_ERROR(x) \
    {                    \
    }
#define SP_APP_ERROR(x) \
    {                   \
    }

#endif