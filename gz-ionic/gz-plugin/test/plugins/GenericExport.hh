#ifndef GZ_PLUGIN_TEST_GENERICEXPORT_HH_
#define GZ_PLUGIN_TEST_GENERICEXPORT_HH_

#if defined _WIN32 || defined __CYGWIN__
  #ifdef __GNUC__
    #define EXPORT __attribute__ ((dllexport))
  #else
    #define EXPORT __declspec(dllexport)
  #endif
#else
  #if __GNUC__ >= 4
    #define EXPORT __attribute__ ((visibility ("default")))
  #else
    #define EXPORT
  #endif
#endif

#endif
