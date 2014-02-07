Matlab BSON
===========

Matlab BSON encoder based on [libbson](https://github.com/mongodb/libbson).

[BSON](http://bsonspec.org/) is a bin­ary-en­coded seri­al­iz­a­tion of
JSON-like doc­u­ments. This package contains API to convert Matlab variables
to and from BSON binary. It is useful for exchanging data with Matlab and
other programs written in different language.

Build
-----

A UNIX environment is required to build the package. Get necessary tools to
build libbson (automake, autoconf, libtool, gcc, make). If you don't have
libbson installed in the system, you need the Internet connection to download
the libbson package. Also `mex -setup` if you have never used `mex` command in
Matlab.

Once all the requirements are met, type the following in Matlab to build. This
will automatically download and compile the libbson package to build the Matlab
API.

```Matlab
addpath /path/to/matlab-bson;
bson.make;
```

Check `help bson.make` for the detailed instruction.

Example
-------

```Matlab
parrot = struct( ...
  'name', 'Grenny', ...
  'type', 'African Grey', ...
  'male', true, ...
  'age', 1, ...
  'birthdate', {bson.datetime(now)}, ...
  'likes', {{'green color', 'night', 'toys'}}, ...
  'extra1', [] ...
  );
bson_value = bson.encode(parrot);
assert(bson.validate(bson_value));
parrot = bson.decode(bson_value);
disp(bson.asJSON(bson_value));
```

API
---

    asJSON    Convert BSON to JSON.
    datetime  Datetime type in BSON format.
    decode    Deserialize value from BSON format.
    encode    Serialize value in BSON format.
    make      Build a driver mex file.
    validate  Validates BSON format.
