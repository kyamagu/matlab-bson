classdef datetime
%DATETIME Datetime type in BSON format.

  properties (SetAccess = private)
    number % Matlab date number.
  end

  methods
    function this = datetime(varargin)
      %DATE Create a new date object.
      %
      %    date_value = bson.datetime
      %    date_value = bson.datetime('2007-01-01')
      %
      % See also datenum
      if nargin > 0
        if isstruct(varargin{1})
          this = repmat(this, size(varargin{1}));
          [this.number] = deal(varargin{1}.number);
        else
          this.number = datenum(varargin{:});
        end
      else
        this.number = now;
      end
    end
    
    function value = string(this, varargin)
      %STRING Convert to a string representation.
      %
      %    date_value.string('yyyy-mm-dd')
      %
      % See also datestr
      value = datestr([this.number], varargin{:});
    end
    
    function value = vector(this, varargin)
      %STRING Convert to a vector representation.
      %
      %    date_value.vector
      %
      % See also datevec
      value = datevec([this.number], varargin{:});
    end
    
    function disp(this)
      %DISP Display function.
      disp(char(this));
    end
    
    function value = double(this)
      %DOUBLE Convert to double.
      %
      %    double(date_value)
      %
      % See also datenum
      value = double(reshape([this.number], size(this)));
    end
    
    function value = char(this)
      %CHAR Convert to char.
      %
      %    char(date_value)
      %
      value = datestr([this.number]);
    end
    
    function value = struct(this)
      %CHAR Convert to char.
      %
      %    char(date_value)
      %
      value = reshape(struct('number', {this.number}), size(this));
    end
  end

end