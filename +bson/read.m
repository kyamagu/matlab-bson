function value = read(filename, varargin)
%READ Read bson from file.
%
%    value = bson.read(filename)
%
% Parameters:
%
%    - `filename` Path to the BSON file.
%
% Returns:
%
%    Matlab value.
%
% See also bson
  fid = fopen(filename, 'r');
  assert(fid > 0, 'Invalid file: %s', filename);
  try
    bson_binary = fread(fid, inf, 'uint8=>uint8');
    fclose(fid);
  catch exception
    fclose(fid);
    rethrow(exception);
  end
  value = bson.decode(bson_binary, varargin{:});
end
