function json_value = asJSON(bson_value, varargin)
%ASJSON Convert BSON to JSON.
%
%    json_value = bson.asJSON(bson_value, ...)
%
% Parameters:
%
%    - `bson_value` BSON encoded binary.
%
% Returns:
%
%    JSON string.
%
% See also bson
  json_value = libbsonmex(mfilename, bson_value, varargin{:});
end
