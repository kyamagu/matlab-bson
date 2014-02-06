function bson_value = encode(value, varargin)
%ENCODE Serialize value in BSON format.
%
%    bson_value = bson.encode(value, ...)
%
% Parameters:
%
%    - `bson_value` A value to be encoded as a BSON binary.
%
% Returns:
%
%    A BSON binary.
%
% See also bson
  bson_value = libbsonmex(mfilename, value, varargin{:});
end
