/// Matlab bson API.
///
/// Kota Yamaguchi 2013

#include "bsonmex.h"
#include <mex.h>
#include "mex/arguments.h"
#include "mex/function.h"
#include <stdlib.h>
#include <string.h>

using mex::CheckInputArguments;
using mex::CheckOutputArguments;
using mex::VariableInputArguments;

namespace {

/** Create bson_t value from mxArray.
 * @param input mxArray from which to get a BSON.
 * @return bson_t* value. Caller must destroy the returned bson.
 */
bson_t* CreateBSON(const mxArray* input) {
  const bson_uint8_t* data = reinterpret_cast<bson_uint8_t*>(mxGetData(input));
  bson_t* value = bson_new_from_data(data, mxGetNumberOfElements(input));
  if (!value)
    mexErrMsgIdAndTxt("bsonmex:error", "Invalid BSON data.");
  return value;
}

MEX_FUNCTION(encode) (int nlhs,
                      mxArray *plhs[],
                      int nrhs,
                      const mxArray *prhs[]) {
  CheckInputArguments(1, 1, nrhs);
  CheckOutputArguments(0, 1, nlhs);
  bson_t value;
  if (!ConvertMxArrayToBSON(prhs[0], &value))
    mexErrMsgIdAndTxt("bsonmex:error", "Failed to convert.");
  int size = value.len;
  plhs[0] = mxCreateNumericMatrix(1, size, mxUINT8_CLASS, mxREAL);
  memcpy(mxGetData(plhs[0]), bson_get_data(&value), size);
  bson_destroy(&value);
}

MEX_FUNCTION(decode) (int nlhs,
                      mxArray *plhs[],
                      int nrhs,
                      const mxArray *prhs[]) {
  CheckInputArguments(1, 1, nrhs);
  CheckOutputArguments(0, 1, nlhs);
  bson_t* value = CreateBSON(prhs[0]);
  if (!ConvertBSONToMxArray(value, &plhs[0])) {
    bson_destroy(value);
    mexErrMsgIdAndTxt("bsonmex:error", "Failed to convert.");
  }
  bson_destroy(value);
}

MEX_FUNCTION(validate) (int nlhs,
                        mxArray *plhs[],
                        int nrhs,
                        const mxArray *prhs[]) {
  CheckInputArguments(1, 1, nrhs);
  CheckOutputArguments(0, 1, nlhs);
  bson_t* value = CreateBSON(prhs[0]);
  plhs[0] = mxCreateLogicalScalar(bson_validate(value,
                                                BSON_VALIDATE_NONE,
                                                NULL));
  bson_destroy(value);
}

MEX_FUNCTION(asJSON) (int nlhs,
                      mxArray *plhs[],
                      int nrhs,
                      const mxArray *prhs[]) {
  CheckInputArguments(1, 1, nrhs);
  CheckOutputArguments(0, 1, nlhs);
  bson_t* value = CreateBSON(prhs[0]);
  char* json_value = bson_as_json(value, NULL);
  plhs[0] = mxCreateString(json_value);
  bson_free(json_value);
  bson_destroy(value);
}

} // namespace
