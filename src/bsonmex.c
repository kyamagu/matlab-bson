/** mxArray BSON encoder implementation.
 *
 * TODO: Implement sparse array conversion.
 *
 * Kota Yamaguchi 2013
 */

#include "bsonmex.h"
#include <ctype.h>
#include <mex.h>
#include <stdlib.h>
#include <string.h>

static bool ConvertArrayToBSON(const mxArray* input,
                               const char* name,
                               bson_t* output);
static mxArray* ConvertNextToMxArray(bson_iter_t* it);
static mxArray* Convert2DOrNDArrayToCellArray(const mxArray* input);

/** Convert mxArray to BSON binary.
 */
static bool ConvertBinaryArrayToBSON(const mxArray* input,
                                     const char* name,
                                     bson_t* output) {
  size_t num_elements = mxGetNumberOfElements(input);
  uint8_t* values = (uint8_t*)mxGetData(input);
  int length = 0;
  const bson_uint8_t* data;
  if (num_elements == 0)
    return BSON_APPEND_NULL(output, (name) ? name : "0") == TRUE;
  data = (const bson_uint8_t*)mxGetData(input);
  length = mxGetNumberOfElements(input);
  return BSON_APPEND_BINARY(output,
                            (name) ? name : "0",
                            BSON_SUBTYPE_BINARY,
                            data,
                            length) == TRUE;
}

/** Convert mxArray to BSON int array.
 */
static bool ConvertShortArrayToBSON(const mxArray* input,
                                    const char* name,
                                    bson_t* output) {
  char key[16];
  size_t num_elements = mxGetNumberOfElements(input);
  int16_t* values = (int16_t*)mxGetData(input);
  bson_t array;
  int i;
  if (num_elements == 0)
    return BSON_APPEND_NULL(output, (name) ? name : "0") == TRUE;
  if (num_elements == 1)
    return BSON_APPEND_INT32(output, (name) ? name : "0", values[0]) ==
           TRUE;
  if (name && bson_append_array_begin(output,
                                      name,
                                      (int)strlen(name),
                                      &array) != TRUE)
    return false;
  for (i = 0; i < num_elements; ++i) {
    if (sprintf(key, "%d", i) < 0)
      return false;
    if (BSON_APPEND_INT32((name) ? &array : output, key, values[i]) != TRUE)
      return false;
  }
  if (name && bson_append_array_end(output, &array) != TRUE)
    return false;
  return true;
}

/** Convert mxArray to BSON int array.
 */
static bool ConvertIntegerArrayToBSON(const mxArray* input,
                                      const char* name,
                                      bson_t* output) {
  char key[16];
  size_t num_elements = mxGetNumberOfElements(input);
  int32_t* values = (int32_t*)mxGetData(input);
  bson_t array;
  int i;
  if (num_elements == 0)
    return BSON_APPEND_NULL(output, (name) ? name : "0") == TRUE;
  if (num_elements == 1)
    return BSON_APPEND_INT32(output, (name) ? name : "0", values[0]) ==
           TRUE;
  if (name && bson_append_array_begin(output,
                                      name,
                                      (int)strlen(name),
                                      &array) != TRUE)
    return false;
  for (i = 0; i < num_elements; ++i) {
    if (sprintf(key, "%d", i) < 0)
      return false;
    if (BSON_APPEND_INT32((name) ? &array : output, key, values[i]) != TRUE)
      return false;
  }
  if (name && bson_append_array_end(output, &array) != TRUE)
    return false;
  return true;
}

/** Convert mxArray to BSON long array.
 */
static bool ConvertLongArrayToBSON(const mxArray* input,
                                   const char* name,
                                   bson_t* output) {
  char key[16];
  size_t num_elements = mxGetNumberOfElements(input);
  int64_t* values = (int64_t*)mxGetData(input);
  bson_t array;
  int i;
  if (num_elements == 0)
    return BSON_APPEND_NULL(output, (name) ? name : "0") == TRUE;
  if (num_elements == 1)
    return BSON_APPEND_INT64(output, (name) ? name : "0", values[0]) ==
           TRUE;
  if (name && bson_append_array_begin(output,
                                      name,
                                      (int)strlen(name),
                                      &array) != TRUE)
    return false;
  for (i = 0; i < num_elements; ++i) {
    if (sprintf(key, "%d", i) < 0)
      return false;
    if (BSON_APPEND_INT64((name) ? &array : output, key, values[i]) != TRUE)
      return false;
  }
  if (name && bson_append_array_end(output, &array) != TRUE)
    return false;
  return true;
}

/** Convert mxArray to BSON bool array.
 */
static bool ConvertLogicalArrayToBSON(const mxArray* input,
                                      const char* name,
                                      bson_t* output) {
  char key[16];
  size_t num_elements = mxGetNumberOfElements(input);
  mxLogical* values = mxGetLogicals(input);
  bson_t array;
  int i;
  if (num_elements == 0)
    return BSON_APPEND_NULL(output, (name) ? name : "0") == TRUE;
  if (num_elements == 1)
    return BSON_APPEND_BOOL(output, (name) ? name : "0", values[0]) ==
           TRUE;
  if (name && bson_append_array_begin(output,
                                      name,
                                      (int)strlen(name),
                                      &array) != TRUE)
    return false;
  for (i = 0; i < num_elements; ++i) {
    if (sprintf(key, "%d", i) < 0)
      return false;
    if (BSON_APPEND_BOOL((name) ? &array : output, key, values[i]) != TRUE)
      return false;
  }
  if (name && bson_append_array_end(output, &array) != TRUE)
    return false;
  return true;
}

/** Convert mxArray to BSON string.
 */
static bool ConvertCharArrayToBSON(const mxArray* input,
                                   const char* name,
                                   bson_t* output) {
  mxArray* prhs[] = {(mxArray*)input, mxCreateString("UTF-8")};
  mxArray* converted_input = NULL;
  size_t length;
  char* value;
  bool status;
  mexCallMATLAB(1, &converted_input, 2, prhs, "unicode2native");
  length = mxGetNumberOfElements(converted_input);
  value = (char*)mxGetData(converted_input);
  if (length && !value)
    return false;
  status = bson_append_utf8(output,
                            (name) ? name : "0",
                            (int)strlen((name) ? name : "0"),
                            value,
                            length) == TRUE;
  mxDestroyArray(prhs[1]);
  mxDestroyArray(converted_input);
  return status;
}

/** Convert mxArray to BSON double array.
 */
static bool ConvertFloatArrayToBSON(const mxArray* input,
                                    const char* name,
                                    bson_t* output) {
  char key[16];
  size_t num_elements = mxGetNumberOfElements(input);
  float* values = (float*)mxGetData(input);
  bson_t array;
  int i;
  if (num_elements == 0)
    return BSON_APPEND_NULL(output, (name) ? name : "0") == TRUE;
  if (num_elements == 1)
    return BSON_APPEND_DOUBLE(output, (name) ? name : "0", values[0]) ==
           TRUE;
  if (name && bson_append_array_begin(output,
                                      name,
                                      (int)strlen(name),
                                      &array) != TRUE)
    return false;
  for (i = 0; i < num_elements; ++i) {
    if (sprintf(key, "%d", i) < 0)
      return false;
    if (BSON_APPEND_DOUBLE((name) ? &array : output, key, values[i]) != TRUE)
      return false;
  }
  if (name && bson_append_array_end(output, &array) != TRUE)
    return false;
  return true;
}

/** Convert mxArray to BSON double array.
 */
static bool ConvertDoubleArrayToBSON(const mxArray* input,
                                     const char* name,
                                     bson_t* output) {
  char key[16];
  size_t num_elements = mxGetNumberOfElements(input);
  double* values = mxGetPr(input);
  bson_t array;
  int i;
  if (num_elements == 0)
    return BSON_APPEND_NULL(output, (name) ? name : "0") == TRUE;
  if (num_elements == 1)
    return BSON_APPEND_DOUBLE(output, (name) ? name : "0", values[0]) ==
           TRUE;
  if (name && bson_append_array_begin(output,
                                      name,
                                      (int)strlen(name),
                                      &array) != TRUE)
    return false;
  for (i = 0; i < num_elements; ++i) {
    if (sprintf(key, "%d", i) < 0)
      return false;
    if (BSON_APPEND_DOUBLE((name) ? &array : output, key, values[i]) != TRUE)
      return false;
  }
  if (name && bson_append_array_end(output, &array) != TRUE)
    return false;
  return true;
}

/** Convert mxArray to BSON date array.
 */
static bool ConvertDateArrayToBSON(const mxArray* input,
                                   const char* name,
                                   bson_t* output) {
  char key[16];
  size_t num_elements = mxGetNumberOfElements(input);
  bson_t array;
  int i;
  if (num_elements == 0)
    return BSON_APPEND_NULL(output, (name) ? name : "0") == TRUE;
  if (num_elements == 1) {
    mxArray* value = mxGetProperty(input, 0, "number");
    bson_int64_t date_value;
    if (!value)
      return false;
    date_value = (bson_int64_t)((mxGetScalar(value) - 719529) * 86400);
    return BSON_APPEND_DATE_TIME(output, (name) ? name : "0", date_value) ==
           TRUE;
  }
  if (name && bson_append_array_begin(output,
                                      name,
                                      (int)strlen(name),
                                      &array) != TRUE)
    return false;
  for (i = 0; i < num_elements; ++i) {
    mxArray* value;
    bson_int64_t date_value;
    if (sprintf(key, "%d", i) < 0)
      return false;
    value = mxGetProperty(input, i, "number");
    if (!value)
      return false;
    date_value = (bson_int64_t)((mxGetScalar(value) - 719529) * 86400);
    if (BSON_APPEND_DATE_TIME((name) ? &array : output,
                              key,
                              date_value) != TRUE)
      return false;
  }
  if (name && bson_append_array_end(output, &array) != TRUE)
    return false;
  return true;
}

/** Convert cell mxArray to BSON array.
 */
static bool ConvertCellArrayToBSON(const mxArray* input,
                                   const char* name,
                                   bson_t* output) {
  char key[16];
  size_t num_elements = mxGetNumberOfElements(input);
  bson_t array;
  int i;
  if (name && bson_append_array_begin(output,
                                      name,
                                      (int)strlen(name),
                                      &array) != TRUE)
    return false;
  for (i = 0; i < num_elements; ++i) {
    mxArray* element = mxGetCell(input, i);
    if (sprintf(key, "%d", i) < 0)
      return false;
    if (!ConvertArrayToBSON(element, key, (name) ? &array : output))
      return false;
  }
  if (name && bson_append_array_end(output, &array) != TRUE)
    return false;
  return true;
}

/** Check if oid is given.
 */
static bool ConvertStringToOID(const mxArray* element,
                               bson_t* output) {
  char* value = mxArrayToString(element);
  bson_oid_t oid;
  bson_oid_init_from_string(&oid, value);
  mxFree(value);
  return BSON_APPEND_OID(output, "_id", &oid) == TRUE;
}

/** Convert struct mxArray to BSON array.
 */
static bool ConvertStructArrayToBSON(const mxArray* input,
                                     const char* name,
                                     bson_t* output) {
  size_t num_elements = mxGetNumberOfElements(input);
  int num_fields = mxGetNumberOfFields(input);
  bson_t array;
  int i, j;
  if (num_elements == 1) {
    bson_t document;
    if (name && bson_append_document_begin(output,
                                           name,
                                           (int)strlen(name),
                                           &document) != TRUE)
      return false;
    for (i = 0; i < num_fields; ++i) {
      mxArray* element = mxGetFieldByNumber(input, 0, i);
      const char* field_name = mxGetFieldNameByNumber(input, i);
      /* Convert string to OID only if a scalar struct with id field. */
      if (name == NULL &&
          strcmp(field_name, "id_") == 0 &&
          mxIsChar(element) &&
          mxGetNumberOfElements(element) == 12) {
        if (!ConvertStringToOID(element, (name) ? &document : output))
          return false;
      }
      else
        if (!ConvertArrayToBSON(element,
                                field_name,
                                (name) ? &document : output))
          return false;
    }
    if (name && bson_append_document_end(output, &document) != TRUE)
      return false;
  }
  else {
    char key[16];
    if (name && bson_append_array_begin(output,
                                        name,
                                        (int)strlen(name),
                                        &array) != TRUE)
      return false;
    for (j = 0; j < num_elements; ++j) {
      bson_t document;
      if (sprintf(key, "%d", j) < 0)
        return false;
      if (bson_append_document_begin((name) ? &array : output,
                                     key,
                                     (int)strlen(key),
                                     &document) != TRUE)
        return false;
      for (i = 0; i < num_fields; ++i) {
        mxArray* element = mxGetFieldByNumber(input, 0, i);
        const char* field_name = mxGetFieldNameByNumber(input, i);
        if (!ConvertArrayToBSON(element, field_name, &document))
          return false;
      }
      if (bson_append_document_end((name) ? &array : output,
                                   &document) != TRUE)
        return false;
    }
    if (name && bson_append_array_end(output, &array) != TRUE)
      return false;
  }
  return true;
}

/** Convert any ND (>2D) array to a nested cell array.
 */
static mxArray* ConvertNDArrayToCellArray(const mxArray* input,
                                   mwSize ndims,
                                   const mwSize* dims) {
  mwSize last_dimentions = dims[ndims - 1];
  mxArray* array = mxCreateCellMatrix(1, last_dimentions);
  mwSize num_elements = mxGetNumberOfElements(input) / last_dimentions;
  int i, j;
  for (i = 0; i < last_dimentions; ++i) {
    mxArray* element = NULL;
    mxArray* split_element;
    switch (mxGetClassID(input)) {
      case mxSTRUCT_CLASS: {
        int nfields = mxGetNumberOfFields(input);
        const char** fields = (const char**)mxMalloc(
            sizeof(const char*) * nfields);
        int k;
        for (k = 0; k < nfields; ++k)
          fields[k] = mxGetFieldNameByNumber(input, k);
        element = mxCreateStructArray(ndims - 1, dims, nfields, fields);
        for (j = 0; j < num_elements; ++j) {
          mwSize index = j + i * num_elements;
          for (k = 0; k < nfields; ++k) {
            mxArray* value = mxDuplicateArray(mxGetFieldByNumber(input,
                                                                 index,
                                                                 k));
            mxSetFieldByNumber(element, j, k, value);
          }
        }
        mxFree(fields);
        break;
      }
      case mxCELL_CLASS: {
        element = mxCreateCellArray(ndims - 1, dims);
        for (j = 0; j < num_elements; ++j) {
          mwSize index = j + num_elements * i;
          mxSetCell(element, j, mxDuplicateArray(mxGetCell(input, index)));
        }
        break;
      }
      case mxDOUBLE_CLASS:
      case mxINT8_CLASS:
      case mxUINT8_CLASS:
      case mxINT16_CLASS:
      case mxUINT16_CLASS:
      case mxINT32_CLASS:
      case mxUINT32_CLASS:
      case mxINT64_CLASS:
      case mxUINT64_CLASS:
      case mxSINGLE_CLASS: {
        mwSize element_size, stride;
        element = mxCreateNumericArray(ndims - 1,
                                       dims,
                                       mxGetClassID(input),
                                       mxREAL);
        element_size = mxGetElementSize(input);
        stride = element_size * num_elements;
        memcpy((void*)mxGetData(element),
               (void*)mxGetData(input) + i * stride,
               stride);
        break;
      }
      case mxCHAR_CLASS: {
        mwSize element_size, stride;
        element = mxCreateCharArray(ndims - 1, dims);
        element_size = mxGetElementSize(input);
        stride = element_size * num_elements;
        memcpy((void*)mxGetData(element),
               (void*)mxGetData(input) + i * stride,
               stride);
        break;
      }
      case mxLOGICAL_CLASS: {
        mwSize element_size, stride;
        element = mxCreateLogicalArray(ndims - 1, dims);
        element_size = mxGetElementSize(input);
        stride = element_size * num_elements;
        memcpy((void*)mxGetData(element),
               (void*)mxGetData(input) + i * stride,
               stride);
        break;
      }
      case mxOBJECT_CLASS:
      case mxVOID_CLASS:
      case mxFUNCTION_CLASS:
      case mxOPAQUE_CLASS:
      default:
        return NULL;
    }
    if (!element)
      return NULL;
    split_element = Convert2DOrNDArrayToCellArray(element);
    if (!split_element)
      return NULL;
    if (split_element != element)
      mxDestroyArray(element);
    mxSetCell(array, i, split_element);
  }
  return array;
}

/** Convert any 2D array to a cell array of row vectors.
 */
static mxArray* Convert2DArrayToCellArray(const mxArray* input,
                                   mwSize ndims,
                                   const mwSize* dims) {
  mxArray* array = mxCreateCellMatrix(1, dims[0]);
  int i, j;
  for (i = 0; i < dims[0]; ++i) {
    mxArray* element = NULL;
    switch (mxGetClassID(input)) {
      case mxSTRUCT_CLASS: {
        int nfields = mxGetNumberOfFields(input);
        const char** fields = (const char**)mxMalloc(
            sizeof(const char*) * nfields);
        int k;
        for (k = 0; k < nfields; ++k)
          fields[k] = mxGetFieldNameByNumber(input, k);
        element = mxCreateStructMatrix(1, dims[1], nfields, fields);
        for (j = 0; j < dims[1]; ++j) {
          mwSize index = i + j * dims[0];
          for (k = 0; k < nfields; ++k) {
            mxArray* value = mxDuplicateArray(mxGetFieldByNumber(input,
                                                                 index,
                                                                 k));
            mxSetFieldByNumber(element, j, k, value);
          }
        }
        mxFree(fields);
        break;
      }
      case mxCELL_CLASS: {
        element = mxCreateCellMatrix(1, dims[1]);
        for (j = 0; j < dims[1]; ++j) {
          mwSize index = i + j * dims[0];
          mxSetCell(element, j, mxDuplicateArray(mxGetCell(input, index)));
        }
        break;
      }
      case mxDOUBLE_CLASS:
      case mxINT8_CLASS:
      case mxUINT8_CLASS:
      case mxINT16_CLASS:
      case mxUINT16_CLASS:
      case mxINT32_CLASS:
      case mxUINT32_CLASS:
      case mxINT64_CLASS:
      case mxUINT64_CLASS:
      case mxSINGLE_CLASS: {
        mwSize element_size, stride;
        void* input_data;
        void* output_data;
        element = mxCreateNumericMatrix(1,
                                        dims[1],
                                        mxGetClassID(input),
                                        mxREAL);
        element_size = mxGetElementSize(input);
        stride = element_size * dims[0];
        input_data = (void*)mxGetData(input) + i * element_size;
        output_data = (void*)mxGetData(element);
        for (j = 0; j < dims[1]; ++j) {
          memcpy(output_data, input_data, element_size);
          input_data += stride;
          output_data += element_size;
        }
        break;
      }
      case mxCHAR_CLASS: {
        mwSize char_dims[] = {1, dims[1]};
        mwSize element_size, stride;
        void* input_data;
        void* output_data;
        element = mxCreateCharArray(2, char_dims);
        element_size = mxGetElementSize(input);
        stride = element_size * dims[0];
        input_data = (void*)mxGetData(input) + i * element_size;
        output_data = (void*)mxGetData(element);
        for (j = 0; j < dims[1]; ++j) {
          memcpy(output_data, input_data, element_size);
          input_data += stride;
          output_data += element_size;
        }
        break;
      }
      case mxLOGICAL_CLASS: {
        mwSize element_size, stride;
        void* input_data;
        void* output_data;
        element = mxCreateLogicalMatrix(1, dims[1]);
        element_size = mxGetElementSize(input);
        stride = element_size * dims[0];
        input_data = (void*)mxGetData(input) + i * element_size;
        output_data = (void*)mxGetData(element);
        for (j = 0; j < dims[1]; ++j) {
          memcpy(output_data, input_data, element_size);
          input_data += stride;
          output_data += element_size;
        }
        break;
      }
      case mxOBJECT_CLASS:
      case mxVOID_CLASS:
      case mxFUNCTION_CLASS:
      case mxOPAQUE_CLASS:
      default:
        return NULL;
    }
    if (!element)
      return NULL;
    mxSetCell(array, i, element);
  }
  return array;
}

/** Convert any 2D or ND array to a nested cell array.
 */
static mxArray* Convert2DOrNDArrayToCellArray(const mxArray* input) {
  mwSize ndims = mxGetNumberOfDimensions(input);
  const mwSize* dims = mxGetDimensions(input);
  if (ndims <= 2 && (dims[0] <= 1 || dims[1] <= 1)) {
    return (mxArray*)input;
  }
  else if (ndims == 2)
    return Convert2DArrayToCellArray(input, ndims, dims);
  else
    return ConvertNDArrayToCellArray(input, ndims, dims);
}

/** Convert any mxArray to BSON.
 */
static bool ConvertArrayToBSON(const mxArray* input,
                               const char* name,
                               bson_t* output) {
  mxArray* array = Convert2DOrNDArrayToCellArray(input);
  if (!array)
    return false;
  switch (mxGetClassID(array)) {
    case mxDOUBLE_CLASS:
      return ConvertDoubleArrayToBSON(array, name, output);
      break;
    case mxSTRUCT_CLASS:
      return ConvertStructArrayToBSON(array, name, output);
      break;
    case mxCELL_CLASS:
      return ConvertCellArrayToBSON(array, name, output);
      break;
    case mxLOGICAL_CLASS:
      return ConvertLogicalArrayToBSON(array, name, output);
      break;
    case mxCHAR_CLASS:
      return ConvertCharArrayToBSON(array, name, output);
      break;
    case mxINT8_CLASS:
    case mxUINT8_CLASS:
      return ConvertBinaryArrayToBSON(array, name, output);
      break;
    case mxINT16_CLASS:
    case mxUINT16_CLASS:
      return ConvertShortArrayToBSON(array, name, output);
      break;
    case mxINT32_CLASS:
    case mxUINT32_CLASS:
      return ConvertIntegerArrayToBSON(array, name, output);
      break;
    case mxINT64_CLASS:
    case mxUINT64_CLASS:
      return ConvertLongArrayToBSON(array, name, output);
      break;
    case mxSINGLE_CLASS:
      return ConvertFloatArrayToBSON(array, name, output);
      break;
    case mxOBJECT_CLASS:
    case mxVOID_CLASS:
    case mxFUNCTION_CLASS:
    case mxOPAQUE_CLASS:
    default:
      if (mxIsClass(input, "bson.datetime")) {
        return ConvertDateArrayToBSON(array, name, output);
        break;        
      }
      return false;
  }
  if (array != input)
    mxDestroyArray(array);
  return false;
}

/** Check the type and the size of the BSON object.
 */
static void CheckBSONObject(bson_iter_t* it,
                            int* object_size, 
                            const char*** keys,
                            int* array_type) {
  int element_type;
  bool is_first = true;
  bool is_array = true;
  *object_size = 0;
  while (bson_iter_next(it)) {
    bson_type_t type = bson_iter_type(it);
    const char* key;
    const char* key_ptr;
    bool is_digit;
    if (type == BSON_TYPE_EOD)
      break;
    key = bson_iter_key(it);
    (*object_size)++;
    /* Keep key names for a struct array. */
    if (keys) {
      *keys = (const char**)mxRealloc(*keys,
                                      *object_size * sizeof(const char*));
      (*keys)[(*object_size) - 1] = key;
    }
    /* Check if it has an consistent index. */
    key_ptr = key;
    is_digit = true;
    while (*key_ptr != 0)
      is_digit &= (isdigit(*key_ptr++) > 0);
    is_array = (is_digit) ? is_array & (*object_size - 1 == atol(key)) : false;
    /* Check the array type from element. */
    if (array_type) {
      switch (type) {
        case BSON_TYPE_DOUBLE:
          element_type = mxDOUBLE_CLASS;
          break;
        case BSON_TYPE_INT32:
          element_type = mxINT32_CLASS;
          break;
        case BSON_TYPE_INT64:
          element_type = mxINT64_CLASS;
          break;
        case BSON_TYPE_BOOL:
          element_type = mxLOGICAL_CLASS;
          break;
        case BSON_TYPE_UTF8:
          element_type = mxCHAR_CLASS;
          break;
        case BSON_TYPE_BINARY:
          element_type = mxUINT8_CLASS;
          break;
        default:
          element_type = mxCELL_CLASS;
          break;
      }
      if (is_first) {
        *array_type = element_type;
        is_first = false;
      }
      else
        *array_type = (*array_type == element_type) ?
                      element_type : mxCELL_CLASS;
    }
  }
  if (array_type && !is_array)
    *array_type = mxSTRUCT_CLASS;
}

/** Convert BSON array to double mxArray.
 */
static mxArray* ConvertBSONArrayToDoubleArray(bson_iter_t* it, int size) {
  mxArray* element = mxCreateDoubleMatrix(1, size, mxREAL);
  double* output_data;
  if (!element)
    return NULL;
  output_data = mxGetPr(element);
  while (bson_iter_next(it)) {
    bson_type_t type = bson_iter_type(it);
    switch (type) {
      case BSON_TYPE_EOD:
        break;
      case BSON_TYPE_DOUBLE:
        *(output_data++) = bson_iter_double(it);
        break;
      case BSON_TYPE_INT32:
        *(output_data++) = bson_iter_int32(it);
        break;
      case BSON_TYPE_INT64:
        *(output_data++) = bson_iter_int64(it);
        break;
      case BSON_TYPE_BOOL:
        *(output_data++) = bson_iter_bool(it);
        break;
      default:
        mxDestroyArray(element);
        return NULL;
    }
  }
  return element;
}

/** Convert BSON array to integer mxArray.
 */
static mxArray* ConvertBSONArrayToIntegerArray(bson_iter_t* it, int size) {
  mxArray* element = mxCreateNumericMatrix(1, size, mxINT32_CLASS, mxREAL);
  int32_t* output_data;
  if (!element)
    return NULL;
  output_data = (int32_t*)mxGetData(element);
  while (bson_iter_next(it)) {
    bson_type_t type = bson_iter_type(it);
    switch (type) {
      case BSON_TYPE_EOD:
        break;
      case BSON_TYPE_DOUBLE:
        *(output_data++) = bson_iter_double(it);
        break;
      case BSON_TYPE_INT32:
        *(output_data++) = bson_iter_int32(it);
        break;
      case BSON_TYPE_INT64:
        *(output_data++) = bson_iter_int64(it);
        break;
      case BSON_TYPE_BOOL:
        *(output_data++) = bson_iter_bool(it);
        break;
      default:
        mxDestroyArray(element);
        return NULL;
    }
  }
  return element;
}

/** Convert BSON array to long mxArray.
 */
static mxArray* ConvertBSONArrayToLongArray(bson_iter_t* it, int size) {
  mxArray* element = mxCreateNumericMatrix(1, size, mxINT64_CLASS, mxREAL);
  int64_t* output_data;
  if (!element)
    return NULL;
  output_data = (int64_t*)mxGetData(element);
  while (bson_iter_next(it)) {
    bson_type_t type = bson_iter_type(it);
    switch (type) {
      case BSON_TYPE_EOD:
        break;
      case BSON_TYPE_DOUBLE:
        *(output_data++) = bson_iter_double(it);
        break;
      case BSON_TYPE_INT32:
        *(output_data++) = bson_iter_int32(it);
        break;
      case BSON_TYPE_INT64:
        *(output_data++) = bson_iter_int64(it);
        break;
      case BSON_TYPE_BOOL:
        *(output_data++) = bson_iter_bool(it);
        break;
      default:
        mxDestroyArray(element);
        return NULL;
    }
  }
  return element;
}

/** Convert BSON array to logical mxArray.
 */
static mxArray* ConvertBSONArrayToLogicalArray(bson_iter_t* it, int size) {
  mxArray* element = mxCreateLogicalMatrix(1, size);
  mxLogical* output_data;
  if (!element)
    return NULL;
  output_data = mxGetLogicals(element);
  while (bson_iter_next(it)) {
    bson_type_t type = bson_iter_type(it);
    switch (type) {
      case BSON_TYPE_EOD:
        break;
      case BSON_TYPE_DOUBLE:
        *(output_data++) = bson_iter_double(it);
        break;
      case BSON_TYPE_INT32:
        *(output_data++) = bson_iter_int32(it);
        break;
      case BSON_TYPE_INT64:
        *(output_data++) = bson_iter_int32(it);
        break;
      case BSON_TYPE_BOOL:
        *(output_data++) = bson_iter_bool(it);
        break;
      default:
        mxDestroyArray(element);
        return NULL;
    }
  }
  return element;
}

/** Convert BSON array to cell mxArray.
 */
static mxArray* ConvertBSONArrayToCellArray(bson_iter_t* it, int size) {
  mxArray* element = mxCreateCellMatrix(1, size);
  int index = 0;
  bool is_end = false;
  int i;
  if (!element)
    return NULL;
  for (i = 0; i < size; ++i) {
    mxArray* sub_element = ConvertNextToMxArray(it);
    if (!sub_element) {
      mxDestroyArray(element);
      return NULL;
    }
    mxSetCell(element, i, sub_element);
  }
  return element;
}

/** Convert keys to matlab-safe names.
 */
static char** CreateSafeKeys(int size, const char* keys[]) {
  char buffer[64]; /* Matlab's variable can be up to 63 characters. */
  char** safe_keys = (char**)mxMalloc(size * sizeof(char*));
  int i, j;
  for (i = 0; i < size; ++i) {
    const char* input = keys[i];
    char* output = buffer;
    bool duplicated;
    int suffix = 0;
    memset(buffer, 0, 64 * sizeof(char));
    /* Special case: oid maps to "id_" field. */
    if (strcmp(input, "_id") == 0) {
      strcpy(output, "id_");
      output += strlen(output);
    }
    else {
      /* variable name: [a-zA-Z][a-zA-Z0-9_]* */
      /* Trim leading non-alphanumeric. */
      while (*input && !isalnum(*input))
        ++input;
      /* If starting from numeric, prepend 'x'. */
      if (*input && isdigit(*input))
        *output++ = 'x';
      while (*input && output < &buffer[63]) {
        if (isalnum(*input))
          *output++ = *input++;
        else {
          /* Convert any consecutive non-alphanumeric to underscore. */
          *output++ = '_';
          while (*input && !isalnum(*input))
            ++input;
        }
      }
    }
    /* If empty, name it 'x'. */
    if (output == buffer)
      *output++ = 'x';
    /* Check if the name is duplicated. If it is, append a number. */
    do {
      duplicated = false;
      for (j = 0; j < i; ++j)
        duplicated |= (strcmp(safe_keys[j], buffer) == 0);
      if (duplicated) {
        /* Append a suffix if duplicated. */
        char suffix_buffer[32];
        int length = sprintf(suffix_buffer, "%d", suffix++);
        if (output + length < &buffer[31])
          strcpy(output, suffix_buffer);
        else {
          /* No resolution to the name collision... */
          for (j = 0; j < i; ++j)
            mxFree(safe_keys[j]);
          mxFree(safe_keys);
          return NULL;
        }
      }
    } while (duplicated);
    /* Copy the safe key name. */
    safe_keys[i] = (char*)mxCalloc(strlen(buffer)+1, sizeof(char));
    strcpy(safe_keys[i], buffer);
  }
  return safe_keys;
}

/** Delete keys.
 */
static void DestroySafekeys(int size, char** keys) {
  int i;
  for (i = 0; i < size; ++i)
    mxFree(keys[i]);
  mxFree(keys);
}

/** Convert BSON array to struct mxArray.
 */
static mxArray* ConvertBSONArrayToStructArray(bson_iter_t* it,
                                              int size,
                                              const char** keys) {
  char** safe_keys = CreateSafeKeys(size, keys);
  int index = 0;
  mxArray* element;
  int i;
  if (!safe_keys)
    return NULL;
  element = mxCreateStructMatrix(1,
                                 1,
                                 size,
                                 (const char**)safe_keys);
  DestroySafekeys(size, safe_keys);
  if (!element)
    return NULL;
  for (i = 0; i < size; ++i) {
    mxArray* sub_element = ConvertNextToMxArray(it);
    if (!sub_element) {
      mxDestroyArray(element);
      return NULL;
    }
    mxSetFieldByNumber(element, 0, i, sub_element);
  }
  return element;
}

/** Merge cell array of numeric arrays to an N-D numeric array.
 */
static void MergeNumericArrays(mxArray** array) {
  int size = mxGetNumberOfElements(*array);
  mxArray* element = mxGetCell(*array, 0);
  mxClassID class_id = mxGetClassID(element);
  mwSize ndims = mxGetNumberOfDimensions(element);
  const mwSize* dims = mxGetDimensions(element);
  mxArray* new_array = NULL;
  int i;
  if (ndims < 2)
    return;
  if (dims[0] == 1) {
    size_t value_size, element_size;
    /* Stack row vectors. */
    new_array = mxCreateNumericMatrix(size, dims[1], class_id, mxREAL);
    if (!new_array)
      return;
    value_size = mxGetNumberOfElements(element);
    element_size = mxGetElementSize(element);
    for (i = 0; i < size; ++i) {
      mxArray* value = mxGetCell(*array, i);
      void* input = (void*)mxGetData(value);
      void* output = (void*)mxGetData(new_array) + i * element_size;
      int j;
      for (j = 0; j < value_size; ++j) {
        memcpy(output, input, element_size);
        input += element_size;
        output += size * element_size;
      }
    }
  }
  else {
    /* Expand the last dimension. */
    mwSize* new_dims = (mwSize*)mxMalloc((ndims + 1) * sizeof(mwSize));
    size_t stride;
    memcpy(new_dims, dims, ndims * sizeof(mwSize));
    new_dims[ndims] = size;
    new_array = mxCreateNumericArray(ndims + 1, new_dims, class_id, mxREAL);
    mxFree(new_dims);
    if (!new_array)
      return;
    stride = mxGetNumberOfElements(element) * mxGetElementSize(element);
    for (i = 0; i < size; ++i) {
      mxArray* value = mxGetCell(*array, i);
      memcpy(mxGetData(new_array) + i * stride, mxGetData(value), stride);
    }
  }
  mxDestroyArray(*array);
  *array = new_array;
}

/** Merge cell array of cell arrays to an N-D cell array.
 */
static void MergeCellArrays(mxArray** array) {
  int size = mxGetNumberOfElements(*array);
  mxArray* element = mxGetCell(*array, 0);
  mxClassID class_id = mxGetClassID(element);
  mwSize ndims = mxGetNumberOfDimensions(element);
  const mwSize* dims = mxGetDimensions(element);
  mxArray* new_array = NULL;
  int i, j;
  if (ndims < 2)
    return;
  if (ndims == 2 && dims[0] == 1) {
    /* Stack row vectors. */
    new_array = mxCreateCellMatrix(size, dims[1]);
    if (!new_array)
      return;
    for (i = 0; i < size; ++i) {
      mxArray* value = mxGetCell(*array, i);
      for (j = 0; j < dims[1]; ++j)
        mxSetCell(new_array,
                  i + j * size,
                  mxDuplicateArray(mxGetCell(value, j)));
    }
  }
  else {
    /* Expand the last dimension. */
    mwSize* new_dims = (mwSize*)mxMalloc(sizeof(mwSize) * (ndims + 1));
    mwSize element_size;
    memcpy(new_dims, dims, sizeof(mwSize) * ndims);
    new_dims[ndims] = size;
    new_array = mxCreateCellArray(ndims + 1, new_dims);
    mxFree(new_dims);
    element_size = mxGetNumberOfElements(element);
    for (i = 0; i < size; ++i) {
      mxArray* value = mxGetCell(*array, i);
      for (j = 0; j < element_size; ++j) {
        mxSetCell(new_array,
                  j + i * element_size,
                  mxDuplicateArray(mxGetCell(value, j)));
      }
    }
  }
  mxDestroyArray(*array);
  *array = new_array;
}

/** Merge cell array of struct arrays to an N-D cell array.
 */
static void MergeStructArrays(mxArray** array) {
  int size = mxGetNumberOfElements(*array);
  mxArray* element = mxGetCell(*array, 0);
  mxClassID class_id = mxGetClassID(element);
  mwSize ndims = mxGetNumberOfDimensions(element);
  const mwSize* dims = mxGetDimensions(element);
  mxArray* new_array = NULL;
  int num_fields;
  int i, j, k;
  const char** fields;
  bool mergeable = true;
  if (ndims < 2)
    return;
  /* Check if all fields are the same. */
  num_fields = mxGetNumberOfFields(element);
  fields = (const char**)mxMalloc(sizeof(const char*) * num_fields);
  for (i = 0; i < num_fields; ++i)
    fields[i] = mxGetFieldNameByNumber(element, i);
  for (i = 1; i < size; ++i) {
    mxArray* value = mxGetCell(*array, i);
    if (mxGetNumberOfFields(value) == num_fields)
      for (k = 0; k < num_fields; ++k)
        mergeable &= strcmp(fields[k], mxGetFieldNameByNumber(value, k)) == 0;
    else
      mergeable = false;
  }
  if (!mergeable) {
    mxFree(fields);
    return;
  }
  if (ndims == 2 && dims[0] == 1 && dims[1] == 1) {
    /* Concatenate into a row vector. */
    new_array = mxCreateStructMatrix(1, size, num_fields, fields);
    if (!new_array)
      return;
    for (i = 0; i < size; ++i) {
      mxArray* value = mxGetCell(*array, i);
      for (k = 0; k < num_fields; ++k) {
        mxArray* field = mxDuplicateArray(mxGetFieldByNumber(value, 0, k));
        mxSetFieldByNumber(new_array, i, k, field);
      }
    }
  }
  else if (ndims == 2 && dims[0] == 1) {
    /* Stack row vectors. */
    new_array = mxCreateStructMatrix(size, dims[1], num_fields, fields);
    if (!new_array)
      return;
    for (i = 0; i < size; ++i) {
      mxArray* value = mxGetCell(*array, i);
      for (j = 0; j < dims[1]; ++j)
        for (k = 0; k < num_fields; ++k) {
          mxArray* field = mxDuplicateArray(mxGetFieldByNumber(value, j, k));
          mxSetFieldByNumber(new_array, i + j * size, k, field);
        }
    }
  }
  else {
    /* Expand the last dimension. */
    mwSize* new_dims = (mwSize*)mxMalloc(sizeof(mwSize) * (ndims + 1));
    mwSize element_size;
    memcpy(new_dims, dims, sizeof(mwSize) * ndims);
    new_dims[ndims] = size;
    new_array = mxCreateStructArray(ndims + 1, new_dims, num_fields, fields);
    mxFree(new_dims);
    element_size = mxGetNumberOfElements(element);
    for (i = 0; i < size; ++i) {
      mxArray* value = mxGetCell(*array, i);
      for (j = 0; j < element_size; ++j)
        for (k = 0; k < num_fields; ++k) {
          mxArray* field = mxDuplicateArray(mxGetFieldByNumber(value, j, k));
          mxSetFieldByNumber(new_array, j + i * element_size, k, field);
        }
    }
  }
  mxFree(fields);
  mxDestroyArray(*array);
  *array = new_array;
}

/** Merge cell array of bson.datetime arrays to an N-D date array.
 */
static void MergeDateArrays(mxArray** array) {
  int size = mxGetNumberOfElements(*array);
  mxArray* element = mxGetCell(*array, 0);
  mxClassID class_id = mxGetClassID(element);
  mwSize ndims = mxGetNumberOfDimensions(element);
  const mwSize* dims = mxGetDimensions(element);
  mxArray* new_array = NULL;
  int dimension;
  int i;
  if (ndims < 2)
    return;
  dimension = (ndims == 2 && dims[0] == 1 && dims[1] == 1) ? 2 :
              (ndims == 2 && dims[0] == 1) ? 1 : ndims;
  mxArray** rhs = (mxArray**)mxMalloc(sizeof(mxArray*) * (1 + size));
  rhs[0] = mxCreateDoubleScalar(2);
  for (i = 0; i < size; ++i)
    rhs[i + 1] = mxGetCell(*array, i);
  mexCallMATLAB(1, &new_array, (1 + size), rhs, "cat");
  mxDestroyArray(rhs[0]);
  mxFree(rhs);
  mxDestroyArray(*array);
  *array = new_array;
}

/** Try to merge cell array to N-D array in place.
 * @param array mxArray to be merged into N-D.
 */
static void TryMergeCellToNDArray(mxArray** array) {
  int size = mxGetNumberOfElements(*array);
  mxArray* element;
  mxClassID class_id;
  mwSize ndims;
  const mwSize* dims;
  bool mergeable = true;
  int i;
  if (!size)
    return;
  /* Get the type information about the first element. */
  element = mxGetCell(*array, 0);
  class_id = mxGetClassID(element);
  ndims = mxGetNumberOfDimensions(element);
  dims = mxGetDimensions(element);
  /* Scan the rest of elements. */
  for (i = 1; i < size; ++i) {
    element = mxGetCell(*array, i);
    mergeable &= class_id == mxGetClassID(element) &&
        ndims == mxGetNumberOfDimensions(element) &&
        memcmp(dims, mxGetDimensions(element), ndims * sizeof(mwSize)) == 0;
  }
  if (!mergeable)
    return;
  switch (class_id) {
    case mxDOUBLE_CLASS:
    case mxLOGICAL_CLASS:
      MergeNumericArrays(array);
      break;
    /*case mxCHAR_CLASS: Let's not merge strings to an N-D array. */
    case mxCELL_CLASS:
      MergeCellArrays(array);
      break;
    case mxSTRUCT_CLASS:
      MergeStructArrays(array);
      break;
    default:
      if (mxIsClass(element, "bson.datetime"))
        MergeDateArrays(array);
      break;
  }
}

/** Convert bson iterator to mxArray*. The iterator must be pointing to a BSON
 * array.
 * @param it bson iterator to convert to mxArray.
 * @param output mxArray to be created.
 * @return Newly allocated mxArray, or NULL if unsuccessful.
 */
static mxArray* ConvertBSONIteratorToMxArray(bson_iter_t* it) {
  mxArray* element = NULL;
  bson_iter_t iterator_copy = *it;
  /* Check the array type. */
  int object_size, array_type;
  const char** keys = NULL;
  CheckBSONObject(&iterator_copy, &object_size, &keys, &array_type);
  if (!keys)
    return NULL;
  /* Convert. */
  switch (array_type) {
    case mxDOUBLE_CLASS:
      element = ConvertBSONArrayToDoubleArray(it, object_size);
      break;
    case mxINT32_CLASS:
      element = ConvertBSONArrayToIntegerArray(it, object_size);
      break;
    case mxINT64_CLASS:
      element = ConvertBSONArrayToLongArray(it, object_size);
      break;
    case mxLOGICAL_CLASS:
      element = ConvertBSONArrayToLogicalArray(it, object_size);
      break;
    case mxCHAR_CLASS:
    case mxUINT8_CLASS:
      if (object_size == 1)
        element = ConvertNextToMxArray(it);
      else
        element = ConvertBSONArrayToCellArray(it, object_size);
      break;
    case mxCELL_CLASS:
      element = ConvertBSONArrayToCellArray(it, object_size);
      break;
    case mxSTRUCT_CLASS:
      element = ConvertBSONArrayToStructArray(it, object_size, keys);
      break;
    default:
      break;
  }
  mxFree(keys);
  /* Merge a cell array to N-D array if possible. */
  if (array_type == mxCELL_CLASS)
    TryMergeCellToNDArray(&element);
  return element;
}

/** Proceed to next and Convert a BSON value.
 */
static mxArray* ConvertNextToMxArray(bson_iter_t* it) {
  mxArray* element = NULL;
  bson_type_t type;
  if (!bson_iter_next(it))
    return NULL;
  type = bson_iter_type(it);
  switch (type) {
    case BSON_TYPE_EOD:
      break;
    case BSON_TYPE_DOUBLE:
      element = mxCreateDoubleScalar(bson_iter_double(it));
      break;
    case BSON_TYPE_UTF8:
    case BSON_TYPE_SYMBOL: {
      bson_uint32_t length = 0;
      const char* value = bson_iter_utf8(it, &length);
      element = mxCreateString(value);
      break;
    }
    case BSON_TYPE_DOCUMENT:
    case BSON_TYPE_ARRAY: {
      bson_iter_t sub_iterator;
      bson_iter_recurse(it, &sub_iterator);
      element = ConvertBSONIteratorToMxArray(&sub_iterator);
      break;
    }
    case BSON_TYPE_BINARY: {
      bson_subtype_t subtype;
      bson_uint32_t element_size;
      const bson_uint8_t *binary;
      bson_iter_binary(it, &subtype, &element_size, &binary);
      element = mxCreateNumericMatrix(1,
                                      element_size,
                                      mxUINT8_CLASS,
                                      mxREAL);
      memcpy(mxGetData(element), binary, element_size);
      break;
    }
    case BSON_TYPE_UNDEFINED:
      element = mxCreateDoubleScalar(mxGetNaN());
      break;
    case BSON_TYPE_OID: {
      char oid_string[25];
      bson_oid_to_string(bson_iter_oid(it), oid_string);
      element = mxCreateString(oid_string);
      break;
    }
    case BSON_TYPE_BOOL:
      element = mxCreateLogicalScalar(bson_iter_bool(it));
      break;
    case BSON_TYPE_DATE_TIME: {
      bson_int64_t date_value = bson_iter_date_time(it);
      mxArray* date_number = mxCreateDoubleScalar(
          ((double)date_value / 86400.0) + 719529);
      mexCallMATLAB(1, &element, 1, &date_number, "bson.datetime");
      mxDestroyArray(date_number);
      break;
    }
    case BSON_TYPE_NULL:
    case BSON_TYPE_DBPOINTER:
      element = mxCreateDoubleMatrix(0, 0, mxREAL);
      break;
    case BSON_TYPE_REGEX:
      element = mxCreateString(bson_iter_regex(it, NULL));
      break;
    case BSON_TYPE_CODE:
    case BSON_TYPE_CODEWSCOPE: {
      bson_uint32_t length;
      element = mxCreateString(bson_iter_code(it, &length));
      break;
    }
    case BSON_TYPE_INT32:
      element = mxCreateDoubleScalar(bson_iter_int32(it));
      break;
    case BSON_TYPE_TIMESTAMP: {
      time_t time_value = bson_iter_time_t(it);
      mxArray* date_number = mxCreateDoubleScalar(
          ((double)time_value / 86400.0) + 719529);
      mexCallMATLAB(1, &element, 1, &date_number, "bson.datetime");
      mxDestroyArray(date_number);
      break;
    }
    case BSON_TYPE_INT64:
      element = mxCreateNumericMatrix(1, 1, mxINT64_CLASS, mxREAL);
      *(int64_t*)mxGetData(element) = bson_iter_int64(it);
      break;
    case BSON_TYPE_MAXKEY:
      element = mxCreateDoubleScalar(mxGetInf());
      break;
    case BSON_TYPE_MINKEY:
      element = mxCreateDoubleScalar(-mxGetInf());
      break;
    default:
      break;
  }
  return element;
}

EXTERN_C bool ConvertMxArrayToBSON(const mxArray* input, bson_t* output) {
  bson_init(output);
  if (!ConvertArrayToBSON(input, NULL, output)) {
    bson_destroy(output);
    return false;
  }
  return true;
}

EXTERN_C bool ConvertBSONToMxArray(const bson_t* input, mxArray** output) {
  bson_iter_t it;
  if (bson_iter_init(&it, input) == TRUE)
    *output = ConvertBSONIteratorToMxArray(&it);
  else
    *output = mxCreateDoubleMatrix(0, 0, mxREAL);
  return *output != NULL;
}