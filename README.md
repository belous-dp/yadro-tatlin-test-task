# Tape sorting
A utility for sorting tapes with numbers. Tapes are simulated using regular text files.

# Building
1. Clone the repo
2. Verify that `VCPKG_DIR` variable is set and it points to vcpkg installation directory.
3. Modify [CMakePresets.json](CMakePresets.json) if you need (for Windows+MVSC or Linux+gcc you don't need to do anything).
4. Select one of the presets and build target `tape_sorting`

# Usage

```
    ./tape_sorting input count {OPTIONS}

  OPTIONS:

      -h, --help                        Display this help menu
      -p, --print                       Whether to print the output tape to
                                        stdout
      input                             Path to the file that emulates input
                                        tape.
      count                             Number of elements to sort.
      -c[cutoff], --cutoff=[cutoff]     The number of elements that can be
                                        sorted in RAM.
      -o[output], --output=[output],
      --dst=[output]                    Path to the output tape. It either
                                        should be correct tape or should be an
                                        empty file,or should point to the
                                        desired location of the output tape.
      --config=[config], --cfg=[config] Path to the file tapes config file or to
                                        the desired location where to create it.
      "--" can be used to terminate flag options and force all following
      arguments to be treated as positional options

```

# Internals

Uses modified iterative merge sort algorithm. Splits src into two blocks, stores them on two tapes, 
then merges blocks on these two tapes into blocks of x2 size and stores them onto other two tapes.
To optimize rewinding, alternates between storing blocks in ascending and descending order.

# Notes
Tested on Windows + MinGW-64 (by msys2).
