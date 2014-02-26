/** MEX-disptach: Macro library to build a dispatchable MEX file.
 *
 * Example:
 *
 *   // mylibrary.c
 *   #include "mex-dispatch.h"
 *   void myFunction(int nlhs, mxArray** plhs,
 *                   int nrhs, const mxArray** prhs) {
 *     mexPrintf("myFunction called.");
 *   }
 *
 *   void myFunction2(int nlhs, mxArray** plhs,
 *                    int nrhs, const mxArray** prhs) {
 *     MEX_ASSERT(nrhs > 0, "Missing argument.");
 *     mexPrintf("myFunction2 called: %g.", mxGetScalar(prhs[0]));
 *   }
 *
 *   MEX_DISPATCH_MAIN(
 *     MEX_DISPATCH_ADD(myFunction),
 *     MEX_DISPATCH_ADD(myFunction2)
 *   )
 *
 * In Matlab,
 *
 *   >> mylibrary('myFunction')     % --> myFunction called.
 *   >> mylibrary('myFunction2', 1) % --> myFunction2 called: 1.
 *
 * Note: add a function declaration list if splitting macros from
 * the function implementation.
 *
 *   // mylibrary.c
 *   #include "mex-dispatch.h"
 *   MEX_DISPATCH_DECLARE(myFunction);
 *   MEX_DISPATCH_DECLARE(myFunction2);
 *   MEX_DISPATCH_MAIN(
 *     MEX_DISPATCH_ADD(myFunction),
 *     MEX_DISPATCH_ADD(myFunction2)
 *   );
 *
 *   // mylibrary-impl.c
 *   void myFunction(...) {}
 * 
 */
#ifndef __MEX_DISPATCH_H__
#define __MEX_DISPATCH_H__

#include <mex.h>
#include <string.h>

/** Declare a dispachable function.
 */
#define MEX_DISPATCH_DECLARE(name) \
EXTERN_C void name(int nlhs, mxArray** plhs, int nrhs, const mxArray** prhs)

/** Add a dispatch entry.
 */
#define MEX_DISPATCH_ADD(name) {#name, name}

/** Create a main entry point for MEX interface.
 */
#define MEX_DISPATCH_MAIN(...) \
typedef struct { \
  const char* name; \
  void (*function)(int, mxArray**, int, const mxArray**); \
} function_entry_t; \
static const function_entry_t kFunctionTable[] = {__VA_ARGS__}; \
void mexFunction(int nlhs, mxArray** plhs, int nrhs, const mxArray** prhs) { \
  int i = 0; \
  char function_name[64]; \
  if (nrhs == 0) \
    mexErrMsgIdAndTxt("dispatch:error", "Missing function name."); \
  if (!mxIsChar(prhs[0])) \
    mexErrMsgIdAndTxt("dispatch:error", \
                      "Operation name must be char but %s.", \
                      mxGetClassName(prhs[0])); \
  if (mxGetString(prhs[0], function_name, sizeof(function_name)) != 0) \
    mexErrMsgIdAndTxt("dispatch:error", "Failed to get a function name."); \
  for (i = 0; i < sizeof(kFunctionTable) / sizeof(function_entry_t); ++i) { \
    if (strcmp(kFunctionTable[i].name, function_name) == 0) { \
      kFunctionTable[i].function(nlhs, plhs, nrhs - 1, prhs + 1); \
      return; \
    } \
  } \
  mexErrMsgIdAndTxt("dispatch:error", "Unknown function `%s`.", function_name); \
}

#endif /* __MEX_DISPATCH_H__ */
