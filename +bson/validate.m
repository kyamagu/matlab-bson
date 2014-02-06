function flag = validate(bson_value, varargin)
%VALIDATE Validates the BSON format.
%
%    flag = bson.validate(bson_value, ...)
%
% Parameters:
%
%    - `bson_value` BSON encoded binary.
%
% Returns:
%
%    True if the input is a valid BSON. Otherwise, false.
%
% See also bson
  flag = libbsonmex(mfilename, bson_value, varargin{:});
end
