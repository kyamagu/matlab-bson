/** Matlab bson API.
 *
 * Kota Yamaguchi 2013
 */

#include "bsonmex.h"
#include <mex.h>
#include "mex-dispatch.h"
#include <stdlib.h>

#define MEX_ERROR(...) mexErrMsgIdAndTxt("bsonmex:error", __VA_ARGS__)
#define MEX_ASSERT(condition, ...) if (!(condition)) MEX_ERROR(__VA_ARGS__)

/** Check number of input arguments.
 */
static void CheckInputArguments(int min_args, int max_args, int nrhs) {
  MEX_ASSERT(nrhs >= min_args, "Missing input: %d for %d.", nrhs, min_args);
  MEX_ASSERT(nrhs <= max_args, "Too many input: %d for %d.", nrhs, max_args);
}

/** Check number of output arguments.
 */
static void CheckOutputArguments(int min_args, int max_args, int nlhs) {
  MEX_ASSERT(nlhs >= min_args, "Missing output: %d for %d.", nlhs, min_args);
  MEX_ASSERT(nlhs <= max_args, "Too many output: %d for %d.", nlhs, max_args);
}

/** Create bson_t value from mxArray.
 * @param input mxArray from which to get a BSON.
 * @return bson_t* value. Caller must destroy the returned bson.
 */
static bson_t* CreateBSON(const mxArray* input) {
  const uint8_t* data = (uint8_t*)(mxGetData(input));
  bson_t* value = bson_new_from_data(data, mxGetNumberOfElements(input));
  MEX_ASSERT(value, "Invalid BSON data.");
  return value;
}

/** Encode a matlab variable in BSON.
 */
static void encode(int nlhs, mxArray *plhs[],
                   int nrhs, const mxArray *prhs[]) {
  CheckInputArguments(1, 1, nrhs);
  CheckOutputArguments(0, 1, nlhs);
  bson_t value;
  MEX_ASSERT(ConvertMxArrayToBSON(prhs[0], &value), "Failed to convert.");
  int size = value.len;
  plhs[0] = mxCreateNumericMatrix(1, size, mxUINT8_CLASS, mxREAL);
  memcpy(mxGetData(plhs[0]), bson_get_data(&value), size);
  bson_destroy(&value);
}

/** Decode a matlab variable from BSON.
 */
static void decode(int nlhs, mxArray *plhs[],
                   int nrhs, const mxArray *prhs[]) {
  CheckInputArguments(1, 1, nrhs);
  CheckOutputArguments(0, 1, nlhs);
  bson_t* value = CreateBSON(prhs[0]);
  bool result = ConvertBSONToMxArray(value, &plhs[0]);
  bson_destroy(value);
  MEX_ASSERT(result, "Failed to convert.");
}

/** Check if the input is a valid BSON.
 */
static void validate(int nlhs, mxArray *plhs[],
                     int nrhs, const mxArray *prhs[]) {
  CheckInputArguments(1, 1, nrhs);
  CheckOutputArguments(0, 1, nlhs);
  bson_t* value = CreateBSON(prhs[0]);
  bool result = bson_validate(value, BSON_VALIDATE_NONE, NULL);
  plhs[0] = mxCreateLogicalScalar(result);
  bson_destroy(value);
}

/** Convert BSON to JSON.
 */
static void asJSON(int nlhs, mxArray *plhs[],
                   int nrhs, const mxArray *prhs[]) {
  CheckInputArguments(1, 1, nrhs);
  CheckOutputArguments(0, 1, nlhs);
  bson_t* value = CreateBSON(prhs[0]);
  char* json_value = bson_as_json(value, NULL);
  plhs[0] = mxCreateString(json_value);
  bson_free(json_value);
  bson_destroy(value);
}

MEX_DISPATCH_MAIN(
  MEX_DISPATCH_ADD(encode),
  MEX_DISPATCH_ADD(decode),
  MEX_DISPATCH_ADD(validate),
  MEX_DISPATCH_ADD(asJSON)
)
