# Benchy Fighters
**Benchy fighters** is a simple generic 2D fighting game made for the purpose of being used in benchmarks.
The game lacks both a UI and realtime input reading, taking all the inputs for a match in the form of a json file.

For further instructions on how to use this program, run the following command after compilation:
```bash
benchyFighters --help
```

You can find a repository with Makefiles to compile this project here:
- https://github.com/michiel9797/Michiels-Makefiles/tree/master

A program to create compatible input files can be found here:
- [timestampedKeylogger](https://github.com/michiel9797/timestampedKeylogger)

The libraries to compile and run the following sublibraries are required:
- [nlohmann-json](https://github.com/nlohmann/json)
- [yojimbo](https://github.com/mas-bandwidth/yojimbo)

Additionally, the following library is also required to compile and run this program:
- [libsfml-dev](https://www.sfml-dev.org)
