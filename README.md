Matlab BSON
===========

Matlab BSON encoder based on [libbson](https://github.com/mongodb/libbson).

Build
-----

A UNIX environment is required to use the package. Also `mex -setup` if you
have never used `mex` command in Matlab.

```Matlab
addpath /path/to/matlab-bson;
bson.make;
```

Check `help bson.make` for detail.

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
    validate  Validates the BSON format.
