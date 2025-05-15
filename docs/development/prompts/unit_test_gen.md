Below are my unit test utilities. I will subsequently give you some code and ask you to write unit tests using them.

```
void todo_start(const char* fmt, ...);
void todo_end(void);
void diag(const char* fmt, ...);
void plan(unsigned int num_tests);
#define ok(test, ...) // Implementation redacted
#define eq_num(a, b, ...) // Implementation redacted
#define neq_num(a, b, ...) // Implementation redacted
#define eq_str(a, b, ...) // Implementation redacted
#define neq_str(a, b, ...) // Implementation redacted
#define eq_null(a, ...) // Implementation redacted
#define neq_null(a, ...) // Implementation redacted
#define is(actual, expected, ...) // Implementation redacted
#define skip_start(cond, num_skips, ...) // Implementation redacted
#define skip_end() // Implementation redacted
#define skip(test, ...) // Implementation redacted
#define done_testing() // Implementation redacted
#define lives(...) // Implementation redacted
#define dies(...) // Implementation redacted
```

---

write unit tests for the following code. assume kmalloc and kmemset are basically malloc and memset and that the interrupt macros will be overidden so we dont have to worry about them in the test environment.
You can set up any test resources necessary to test these functions in as much isolation as possible.

model your tests like

static void abc_test(void) { // test code }
static void xyz_test(void) { // test code }

Use c and programming best practices and make sure tests exist for every edge case, failure mode, and happy case. You MUST use the unit test utilities I provided in the previous message.

```
{code}

```
