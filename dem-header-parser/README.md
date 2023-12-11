# dem-header-parser
A small utility program for probing for metadata and validating the format of Half-Life 2 -based demo files.

## Usage
Build and run the utility by running `make && ./readdem`. To parse one of the sample files, after building, run `./readdem samples/(sample_name).dem`.

The output should contain only lines starting with `[info]` or an error if the format was invalid.
