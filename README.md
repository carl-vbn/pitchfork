# pitchfork
POSIX utility to manage background tasks

## Goal
This project was started out of frustration with the limitations of the `screen` utility. Although very useful, screen is not very configurable and is hard to automate.

pitchfork is designed to manage long-running tasks such as webservers and various microservices running on the same machine. It takes in a configuration file that dictates how each task should be run and will then make sure they keep running while allowing both humans and programs to interface with the running setup.

## Usage
To build the `pitchfork` executable, simply run
```bash
make
```

To run pitchfork, you need to supply a configuration file as a positional argument:
```bash
pitchfork pitchfork-testing.yml
```

This will start the process tree described in [pitchfork-testing.yml](pitchfork-testing.yml)

## Completed and planned features
* [x] Run multiple child processes asynchronously
* [x] Capture standard I/O
* [x] Parse YAML
* [x] Configurable command and arguments 
* [x] Configurable environment variables
* [x] Configurable working directory
* [ ] Configurable permissions
* [ ] Configurable logging
* [x] Sockets to connect to a running pitchfork process
* [ ] Automatic restarts
* [ ] Pre and post-execution commands

An example configuration file for a Game server network can be found under [pitchfork-example.yml](pitchfork-example.yml). This file takes advantage of both currently supported and planned features.
