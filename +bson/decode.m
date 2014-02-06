function value = decode(bson_value, varargin)
%DECODE Deserialize value from BSON format.
%
%    value = bson.decode(bson_value, ...)
%
% Parameters:
%
%    - `bson_value` BSON encoded binary.
%
% Returns:
%
%    Decoded Matlab value.
%
% See also bson
  value = libbsonmex(mfilename, bson_value, varargin{:});
end
