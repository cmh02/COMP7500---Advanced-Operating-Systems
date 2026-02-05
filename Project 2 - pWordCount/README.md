# Project 2: pwordcount

Author: Chris Hinkson

Class: COMP7500 - Advanced Operating Systems

## Description

This project creates a C-based system for counting words in a text file. While on its own a trivial task, this project aims to make complex improvements via feature expansion:

* Command line interfacing
* Customization via configuration files
* Multi-process support
* Inter-process communication via Unix pipes
* Word count algorithmetic synchronization

## Running the Program

To run the program from the command line, use the command: `./pwordcount` with the following arguments:

| Command Argument      | Required / Optional | Description                                                                                                                                                                                                                                 |
| --------------------- | ------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| --file or -f          | Required            | The text file to count words in. File must be of type .txt.<br /><br />Example: `--file mytextfile.txt`                                                                                                                                   |
| --help or -h          | Optional            | Command description for help using the command.<br /><br />Example: `--help`                                                                                                                                                              |
| --config or -c        | Optional            | Location of configuration file.<br />Defaults to `../config/pwordcount.config`.<br /><br />Example: `--config ../config/custom.config`                                                                                                  |
| --nprocesses or -n    | Optional            | The number of counter processes to use.<br />Must be an integer greater than 0 but less than the number of available system cores. <br />Can be set in configuration file.<br />Defaults to `1` .<br /><br />Example: `--nprocesses 8` |
| --logdir or -l        | Optional            | Location of log directory.<br />Can be set in configuration file.<br />Defaults to `../logs/.`<br /><br />Example: `--config ../other/logs`                                                                                             |
| --debug_stdout or -s  | Optional            | Whether to send debug output to stdout.<br />Can be set in configuration file.<br />Defaults to `true`.<br /><br />Example: `--debug_stdout false`                                                                                      |
| --debug_log or -d     | Optional            | Whether to send debug output to log file.<br />Can be set in configuration file.<br />Defaults to `true`.<br /><br />Example: `--debug_log false`                                                                                      |
| --buffersize_r or -x  | Optional            | The size of the Reader Module buffer, in bytes.<br /><br />Example: `--buffersize_r 4096`                                                                                                                                                 |
| --buffersize_cm or -y | Optional            | The size of the Counter Manager Module buffer, in bytes.<br /><br />Example: `--buffersize_cm 4096`                                                                                                                                       |
| --buffersize_c or -z  | Optional            | The size of the Counter Module buffer, in bytes.<br /><br />Example: `--buffersize_c 4096`                                                                                                                                                |

## Configuration File

A configuration file is used to provide users with a way of persisting the desired tool arguments. This file is ideally kept located in a top-level `config` directory, although the specific path can be changed at the command-line too. This file contains many of the settings for the tool which are also available at the command line.

When using both command line arguments and the configuration file, the command line arguments will always take precedence. The priority of tool settings, therefore, is as follows from highest priority to lowest priority:

1. Command-Line Arguments/Flags
2. Configuration Options
3. Tool Default Settings

Following this style of override behavior allows for the most dynamic usage to maintain complete controllability. It also enables for a different configuration file to be specified when running the command instead of requiring a static configuration file location or complete recompilation.

As mentioned, in the case that neither a command argument or configuration option are provided, a default value is available. These are currently also set in the configuration file, which is intended to be changed by the end user. These values have been hard-coded to ensure that the program cannot break due to the lack of an argument or option:

- Debug output to the log file defaults to `true`.
- Debug output to the standard output defaults to `true`.
- The number of counter subprocesses defaults to `1`.
- The buffer size for the Reader Module defaults to `4096 bytes`.
- The buffer size for the Counter-Manager Module defaults to `4096 bytes`.
- The buffer size for the Counter Module defaults to `4096 bytes`.
- The location of the configuration file defaults to `../config/pwordcount.config`.
- The location of the logging directory defaults to `../logs/`.

## Multi-Process Edge Count Algorithm

One of the key problems with a multi-process counter is properly counting words when split into chunks.

### The Initial Example

For instance, with a single worker, we are able to easily resume a word between two chunks:

```
[the text is being sp] [lit apart like this]
```

However, when we have multiple processes, the second process would not know that another counter ever saw the beginning of the word. While it could then be tried to throw away the final portion of a chunk, this then fails to account for the situation that an entire word was received in the buffer up to the very last position:

```
[the text is not split]
```

Therefore, a smart algorithm must be used to account for this. Or rather, as we will see, a naive algorithm. Or even rather, a naive module!

### The Counter-Manager

By introducing a new manager between the reader and counter, called the Counter Manager, we can accomodate for these transitions. Since, to the program, a word is nothing more than a continuation of non-whitespace characters, we can make the assumption that, for two chunks, where the first chunk ends in a non-whitespace character and the second chunk begins in a non-whitespace character, that a word must have been split. By adding an intermediary manager, we can detect this, and solve it.

To accomodate for this issue, the manager will scan the ending character of the first chunk. If the ending character is a whitespace, then it will leave the next chunk alone with no adjustments. However, if the ending character is not whitespace, it will remember this as a flag. Then, on the arrival of the next chunk, it will scan the beginning character to examine whether it is whitespace or character. In the case it is the continuation of a word from the previous chunk, it will simply replace the beginning sequential characters (the end of the word from the previous chunk) with whitespace.

This allows each individual counter to not care about whether a word is beginning or ending. It simply counts how many segments of sequential character elements there are in the given buffer.

### Algorithm In Action

Let us consider that we are receiving the following chunks:

```
[this te] [xt has been split apart]
```

Now, visually, we can tell that there are 6 total words. However, an individual counter with no manager would see 2 words in the first chunk followed by 5 words in the second chunk, resulting in a total count of 7 words. To fix this, our manager will scan the last character of the first chunk on arrival, seeing that it is a non-whitespace character:

```
[this te]
```

It will then remember this for the next chunk. When the next chunk arrives, it sees that the second chunk begins with more non-whitespace characters:

```
[xt has been split apart]
```

So, it will adjust this by replacing these characters with whitespace:

```
[this te] [   has been split apart]
```

Then, each counter can ignore outside factors, and simply count how many words its chunk has. The first chunk will have two words, the second chunk will have four words, giving a total of six words, which is correct.
