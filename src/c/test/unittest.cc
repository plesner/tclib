//- Copyright 2014 the Neutrino authors (see AUTHORS).
//- Licensed under the Apache License, Version 2.0 (see LICENSE).

#include "unittest.hh"

// TODO: fix timing for msvc
#ifdef IS_GCC
#include <time.h>
extern char *strdup(const char*);
#endif

// Returns the current time since epoch counted in seconds.
static double get_current_time_seconds() {
  // TODO: fix timing for msvc.
#ifdef IS_GCC
  struct timespec spec;
  clock_gettime(CLOCK_REALTIME, &spec);
  return spec.tv_sec + (spec.tv_nsec / 1000000000.0);
#else
  return 0;
#endif
}

// Match the given suite and test name against the given unit test data, return
// true iff the test should be run.
bool TestCaseInfo::matches(unit_test_selector_t *selector) {
  if (selector->suite == NULL)
    return true;
  if (strcmp(suite, selector->suite) != 0)
    return false;
  if (selector->name == NULL)
    return true;
  return strcmp(name, selector->name) == 0;
}

void TestCaseInfo::run() {
  static const size_t kTimeColumn = 32;
  printf("- %s/%s", suite, name);
  fflush(stdout);
  double start = get_current_time_seconds();
  unit_test();
  double end = get_current_time_seconds();
  for (size_t i = strlen(suite) + strlen(name); i < kTimeColumn; i++)
    putc(' ', stdout);
  printf(" (%.3fs)\n", (end - start));
  fflush(stdout);
}

static void parse_test_selector(const char *str, unit_test_selector_t *selector) {
  char *dupped = strdup(str);
  selector->suite = dupped;
  char *test = strchr(dupped, '/');
  if (test != NULL) {
    test[0] = 0;
    selector->name = &test[1];
  } else {
    selector->name = NULL;
  }
}

static void dispose_test_selector(unit_test_selector_t *selector) {
  free(selector->suite);
}

TestCaseInfo *TestCaseInfo::chain = NULL;

TestCaseInfo::TestCaseInfo(const char *suite, const char *name, unit_test_t unit_test) {
  this->suite = suite;
  this->name = name;
  this->unit_test = unit_test;
  this->next = TestCaseInfo::chain;
  TestCaseInfo::chain = this;
}

void TestCaseInfo::run_tests(unit_test_selector_t *selector) {
  TestCaseInfo *current = TestCaseInfo::chain;
  while (current != NULL) {
    if (current->matches(selector))
      current->run();
    current = current->next;
  }
}

// Run!
int main(int argc, char *argv[]) {
  if (argc >= 2) {
    // If there are arguments run the relevant test suites.
    for (int i = 1; i < argc; i++) {
      unit_test_selector_t selector;
      parse_test_selector(argv[i], &selector);
      TestCaseInfo::run_tests(&selector);
      dispose_test_selector(&selector);
    }
    return 0;
  } else {
    // If there are no arguments just run everything.
    unit_test_selector_t selector = {NULL, NULL};
    TestCaseInfo::run_tests(&selector);
  }
  return 0;
}
