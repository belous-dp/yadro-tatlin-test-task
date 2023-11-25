# Tape sorting
A utility for sorting tapes containing integers.
Tapes are emulated using regular text files.


## Building
1. Clone the repo.
2. Verify that `VCPKG_DIR` variable is set and it points to vcpkg installation directory.
3. \[optional\] If you're using Clang or Windows+MinGW, modify [CMakePresets.json](CMakePresets.json) by your needs.
If you're using Windows+MVSC or Linux+GCC, you don't need to do anything.
4. Select suitable CMake preset and build target `tape_sorting`
5. \[optional\] Build target `tests`


## Usage
### Format of input/output files
[file_tape](file_tape.h) class emulates a tape by interacting with a text file.
The tape data is stored (and parsed) as follows:
<code>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;1&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;123&nbsp;&nbsp;&nbsp;&nbsp;10000000&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-1234567890&nbsp;</code>.
A file with such content represents a tape of size 5: `{ 1, 123, 10000000, [empty], -1234567890 }`.
That is, to store each number,
<11 minus the length of a string representation of the number> whitespaces is written,
then the number is written, then a whitespace is written (to separate elements from each other).
Empty elements are allowed (stored as 11 whitespaces).

### Format of a timings configuration file
To simulate delays in I/O operations of a real tape, timings from [timings_config](file_tape.h) class are used.
Timings are measured in milliseconds and parsed from the config file in the following format:
a field name (a timing name), followed by '=', followed by the field value.
Field names:
* 'r' or "read" for read timing;
* 'w' or "write" for write timing;
* "ml" or "move_left" for move_left timing;
* "mr" or "move_right" for move_right timing;
* "rewind" for "rewind" timing.

Example: `mr=1337 r=123  move_left=42ms rewind=101`.
Result: `{ .read=123, .write=0, .move_left=42, .move_right=1337, .rewind=101 }`.

### CLI
```
    ./tape_sorting input count {OPTIONS}

  OPTIONS:

      -h, --help                        Display this help menu
      -p, --print                       Whether to print the output tape to
                                        stdout
      input                             Path to the file that stores input
                                        tape's data.
      count                             The number of elements to sort. Cannot
                                        be zero.
      -m[cutoff], --cutoff=[cutoff]     The number of elements that can be
                                        sorted in RAM. This number can be less
                                        than 'count' but cannot be zero.The
                                        default value is 1e7.
      --output=[output], --dst=[output] Path to the output tape. It either
                                        should be correct tape or should be an
                                        empty file, or should point to the
                                        desired location of the output tape. The
                                        default value is 'sorted_tape.txt'.
      --config=[config], --cfg=[config] Path to the file tapes config file or to
                                        the desired location where to create it.
                                        The default value is 'file_tape.cfg'.
```

> for reviewers: in terms of the statement.pdf, count = N, cutoff = M.


## Internals

The program uses modified iterative merge sort algorithm:
1. split `src` into blocks of size `cutoff`, and sort them in memory;
2. store sorted blocks evenly on two additional tapes `T1` and `T2`;
3. merge blocks from `T1` and `T2` into blocks of size `cutoff*2`, and stores them onto other two tapes `T3` and `T4`;
4. merge sorted blocks of size `cutoff*2` from `T3` and `T4` and write resulted blocks of size `cutoff*4` to `T1` and `T2`;
5. repeat steps 3 and 4 (doubling size of blocks on each iteration) until we get a fully sorted array.

Optimizations and design decisions:
* [file_tape](file_tape.h) uses 12 bytes per element. Such ineffective (from the point of view of storing data in files)
format was chosen to avoid in-RAM bookkeeping and to allow convenient moving around the file
(and to reduce the number of interactions with cursed C-style I/O API).
* `dst` tape is used as one of `T1..T4` to minimize the number of additional tapes.
* To eliminate rewinding of tapes, the algorithm alternates between merging blocks in ascending and descending order. 
For more information, see [tape_algorithm.cpp](tape_algorithm.cpp).


## Notes
Tested on Windows + MinGW-64 (by msys2).
