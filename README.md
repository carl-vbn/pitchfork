# pitchfork
POSIX utility to manage background tasks

## Goal
This project was started out of frustration with the limitations of the `screen` utility. Although very useful, screen is not very configurable and is hard to automate.

pitchfork is designed to manage long-running tasks such as webservers and various microservices running on the same machine. It takes in a configuration file that dictates how each task should be run and will then make sure they keep running while allowing both humans and programs to interface with the running setup.

## Planned features
* [*] Run multiple child processes asynchronously
* [*] Capture standard I/O
* [*] Parse YAML
* [*] Configurable command and arguments 
* [ ] Configurable environment variables
* [ ] Configurable working directory
* [ ] Configurable permissions
* [ ] Configurable logging
* [ ] Sockets to connect to a running pitchfork process
* [ ] Automatic restarts
* [ ] Pre and post commands