#ifndef _ASSERT_H_
#define _ASSERT_H_
// \see C11 assert.h
#ifndef NDEBUG
#define assert(ignore) ((void)0)

#else // NDEBUG
// It then calls the \ref abort() function
void __aeabi_assert(const char *expr, const char *file, int line);
#define assert(__e) ((__e) ? (void)0 : __aeabi_assert(#__e, __FILE__, __LINE__))
#endif

#endif//_ASSERT_H_

