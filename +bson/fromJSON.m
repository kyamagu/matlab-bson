function bson_value = fromJSON(json_value, varargin)
%FROMJSON Convert JSON to BSON.
%
%    bson_value = bson.asJSON(json_value, ...)
%
% Parameters:
%
%    - `json_value` JSON string.
%
% Returns:
%
%    BSON binary.
%
% See also bson
  bson_value = libbsonmex(mfilename, json_value, varargin{:});
end
