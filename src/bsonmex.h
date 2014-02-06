/** mxArray BSON encoder.
 *
 * Kota Yamaguchi 2013
 */

#ifndef __BSONMEX_H__
#define __BSONMEX_H__

#include "bson.h"
#include <matrix.h>

/** Convert mxArray* to bson.
 * @param input mxArray to convert to bson.
 * @param flags options to change the behavior.
 * @param output bson object to be created. Caller is responsible for calling
 *               bson_destroy() after use. 
 * @return true if success.
 */
EXTERN_C bool ConvertMxArrayToBSON(const mxArray* input, bson_t* output);
/** Convert bson to mxArray*.
 * @param input bson object to convert to mxArray.
 * @param output mxArray to be created.
 * @return true if success.
 */
EXTERN_C bool ConvertBSONToMxArray(const bson_t* input, mxArray** output);

#endif /* __BSONMEX_H__ */
