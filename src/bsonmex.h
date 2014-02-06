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
/** Convert bson iterator to mxArray*. The iterator must be pointing to a BSON
 * array.
 * @param it bson iterator to convert to mxArray.
 * @param output mxArray to be created.
 * @return Newly allocated mxArray, or NULL if unsuccessful.
 */
EXTERN_C mxArray* ConvertBSONIteratorToMxArray(bson_iter_t* it);
/** Try to merge cell array to N-D array in place.
 * @param array mxArray to be merged into N-D.
 */
EXTERN_C void TryMergeCellToNDArray(mxArray** array);

#endif /* __BSONMEX_H__ */
