#include <chrono>

// Example of timing blocks of code within a function:
//
// void my_time_critical_func()
// {
//      _TIMER_START
//      int section, of, critical, code = 0;
//      process(section, of, critical, code);
//      _MILLI_TIMER_FINISH
//      std::cout << "time critical code took " << profile_averageTime << " milliseconds to complete." << std::endl;
// }
//

#define _TIMER_START int profile__totalTime = 0;\
                    auto profile__start = std::chrono::high_resolution_clock::now();

// code to be timed goes here. see also TIMER_WRAP_U|M below.

// millisecond resolution
#define _MILLI_TIMER_FINISH auto profile__end = std::chrono::high_resolution_clock::now();\
                     auto profile__duration = std::chrono::duration_cast<std::chrono::milliseconds>(profile__end - profile__start);\
                     profile__totalTime += profile__duration.count();\
                     double profile__averageTime = static_cast<double>(profile__totalTime);

// microsecond resolution
#define _MICRO_TIMER_FINISH auto profile__end = std::chrono::high_resolution_clock::now();\
                     auto profile__duration = std::chrono::duration_cast<std::chrono::microseconds>(profile__end - profile__start);\
                     profile__totalTime += profile__duration.count();\
                     double profile__averageTime = static_cast<double>(profile__totalTime);


// Complete wrappers for micro- and milli- second resolution function exec profiling.
// Results go in your output_var.
// The macros are contained in their own block so they can be used multiple times
// in the same scope without name clashes.
//
// example:
// void some_func()
// {
//      double my_results;
//      TIMER_WRAP_U(my_func(), my_results)
//
//      double my_other_results;
//      TIMER_WRAP_M(my_other_func(), my_other_results)
//      printf("my_func() took %f microseconds and my_other_func() took %f milliseconds.\n",
//                              my_results,                             my_other_results);
// }

#define TIMER_WRAP_U(code_to_be_timed, output_var) {_TIMER_START (code_to_be_timed);\
                                                    _MICRO_TIMER_FINISH output_var = averageTime;}
// same thing, but with millisecond resolution.
#define TIMER_WRAP_M(code_to_be_timed, output_var) {_TIMER_START (code_to_be_timed);\
                                                    _MILLI_TIMER_FINISH output_var = averageTime;}