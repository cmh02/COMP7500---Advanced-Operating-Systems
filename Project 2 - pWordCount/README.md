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

| Command Argument | Required / Optional | Description                                                                                                                                                                                                                                 |
| ---------------- | ------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| --file           | Required            | The text file to count words in. File must be of type .txt.<br /><br />Example: `--file mytextfile.txt`                                                                                                                                   |
| --config         | Optional            | Location of configuration file.<br />Defaults to `../config/pwordcount.config`.<br /><br />Example: `--config ../config/custom.config`                                                                                                  |
| --nprocesses     | Optional            | The number of counter processes to use.<br />Must be an integer greater than 0 but less than the number of available system cores. <br />Can be set in configuration file.<br />Defaults to `1` .<br /><br />Example: `--nprocesses 8` |
| --logdir         | Optional            | Location of log directory.<br />Can be set in configuration file.<br />Defaults to `../logs/.`<br /><br />Example: `--config ../other/logs`                                                                                             |
| --debug_stdout   | Optional            | Whether to send debug output to stdout.<br />Can be set in configuration file.<br />Defaults to `true`.<br /><br />Example: `--debug_stdout false`                                                                                      |
| --debug_log      | Optional            | Whether to send debug output to log file.<br />Can be set in configuration file.<br />Defaults to `true`.<br /><br />Example: `--debug_log false`                                                                                      |



## Configuration File


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
