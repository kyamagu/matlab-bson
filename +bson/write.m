function write(value, filename, varargin)
%WRITE Write variable to a BSON file.
%
%    bson.write(value, filename)
%
% Parameters:
%
%    - `filename` Path to the BSON file.
%
% See also bson
  bson_binary = bson.encode(value, varargin{:});
  fid = fopen(filename, 'wb');
  assert(fid > 0, 'Invalid file: %s', filename);
  try
    fwrite(fid, bson_binary, 'uint8');
    fclose(fid);
  catch exception
    fclose(fid);
    rethrow(exception);
  end
end
