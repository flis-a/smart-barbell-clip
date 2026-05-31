/* include/app/assert.h
 *
 * Project ASSERT macro - section 6.16.
 * Standard <assert.h> calls abort() which is wrong for embedded; this
 * macro routes failures through assert_failed() instead.
 */
#ifndef APP_ASSERT_H
#define APP_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Called when an ASSERT fires. Defined in src/app/assert.c. */
void assert_failed(const char * file, int line, const char * expr);

#if defined(DEBUG) || !defined(NDEBUG)
    #define ASSERT(x)                                                  \
        do {                                                           \
            if (!(x)) assert_failed(__FILE__, __LINE__, #x);           \
        } while (0)
#else
    #define ASSERT(x) ((void)0)
#endif

/* Use in switch defaults that should be impossible. */
#define ASSERT_UNREACHABLE(msg) assert_failed(__FILE__, __LINE__, (msg))

#ifdef __cplusplus
}
#endif

#endif /* APP_ASSERT_H */