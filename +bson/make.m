function make(varargin)
%MAKE Build a driver mex file.
%
%    bson.make(['optionName', optionValue,] [compiler_flags])
%
% bson.make builds a mex file for the bson driver. The make script
% accepts a few options as well as compiler flags for the mex command.
% See below for the supported build options.
%
% Options:
%
%    Option name         Description
%    ----------------    -------------------------------------------------
%    --download-libbson  logical flag of whether to download bson source.
%    --libbson-version   version number of libbson to download.
%    --libbson-path      path to libejdb.a.
%
% Example:
%
% Default to automatically download the libbson package from GitHub and
% statically link to the libbson binary.
%
% >> bson.make();
%
% Dynamic linking to the system libbson library.
%
% >> bson.make('--download-libbson', false);
%
% Static linking to the user-built libbson binary.
%
% >> bson.make('--download-libbson', false, ...
%              '--libbson-path', '/opt/local/lib/libbson-1.0.a', ...
%              '-I/opt/local/include');
%
% See also mex
  root_dir = fileparts(fileparts(mfilename('fullpath')));
  api_files = { ...
    fullfile(root_dir, 'src', 'libbsonmex.c') ...
    };
  targets = { ...
    fullfile(root_dir, '+bson', 'private', 'libbsonmex') ...
    };
  source_files = setdiff(find_source_files(fullfile(root_dir, 'src')), ...
                         api_files);
  config = parse_options(varargin{:});
  if config.download_libbson
    config = download_libbson(root_dir, config);
  end

  for i = 1:numel(targets)
    [outdir, output] = fileparts(targets{i});
    cmd = sprintf('mex -largeArrayDims%s %s -outdir %s -output %s %s%s', ...
                  sprintf(' %s', source_files{:}), ...
                  api_files{i}, ...
                  outdir, ...
                  output, ...
                  config.libbson_path, ...
                  config.compiler_flags ...
                  );
    disp(cmd);
    eval(cmd);
  end
end

function config = parse_options(varargin)
%PARSE_OPTIONS Parse build options.
  config.download_libbson = true;
  config.libbson_path = '-lbson-1.0';
  config.libbson_version = '0.8.4';
  mark_for_delete = false(size(varargin));
  for i = 1:2:numel(varargin)
    if strcmp(varargin{i}, '--libbson-path')
      config.libbson_path = varargin{i+1};
      mark_for_delete(i:i+1) = true;
    elseif strcmp(varargin{i}, '--libbson-version')
      config.libbson_version = varargin{i+1};
      mark_for_delete(i:i+1) = true;
    elseif strcmp(varargin{i}, '--download-libbson')
      config.download_libbson = logical(varargin{i+1});
      mark_for_delete(i:i+1) = true;
    end
  end
  compiler_flags = sprintf(' %s', varargin{~mark_for_delete});
  if isunix
    compiler_flags = sprintf(' CFLAGS="\\$CFLAGS -fPIC"%s', ...
                             compiler_flags);
    if ~ismac
      compiler_flags = sprintf(' -lrt %s', compiler_flags);
    end
  end
  config.compiler_flags = compiler_flags;
end

function config = download_libbson(root_dir, config)
%DOWNLOAD_EJDB Download the latest EJDB package from GitHub.
  libbson = fullfile(root_dir, sprintf('libbson-%s', config.libbson_version));
  if ~exist(libbson, 'dir')
    github_url = sprintf(...
        'https://github.com/mongodb/libbson/archive/%s.zip', ...
        config.libbson_version);
    zip_file = fullfile(root_dir, ...
                        sprintf('libbson-%s.zip', config.libbson_version));
    disp(['Downloading ', github_url]);
    urlwrite(github_url, zip_file);
    disp(['Extracting ' zip_file]);
    unzip(zip_file, root_dir);
    delete(zip_file);
  end
  config.libbson_path = fullfile(libbson, '.libs', 'libbson-1.0.a');
  if ~exist(config.libbson_path, 'file')
    cmd = sprintf(['cd %s; ./autogen.sh --enable-static; ' ...
                   'make CFLAGS="$CFLAGS -fPIC";'], ...
                  libbson);
    disp(cmd);
    system(cmd);
  end
  config.compiler_flags = sprintf(' -I%s %s', ...
                                  fullfile(libbson, 'src', 'bson'), ...
                                  config.compiler_flags);
end

function files = find_source_files(root_dir)
%SOURCE_FILES List of source files in a string.
  if nargin < 2, excludes = {}; end
  files = dir(root_dir);
  srcs = files(cellfun(@(x)~isempty(x), ...
               regexp({files.name},'\S+\.(c)')));
  srcs = cellfun(@(x)fullfile(root_dir, x), {srcs.name},...
                 'UniformOutput', false);
  subdirs = files([files.isdir] & cellfun(@(x)x(1)~='.',{files.name}));
  subdir_srcs = cellfun(@(x)find_source_files(fullfile(root_dir,x)),...
                        {subdirs.name}, 'UniformOutput', false);
  files = [srcs, subdir_srcs{:}];
end
