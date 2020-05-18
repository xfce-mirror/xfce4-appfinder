## What is it?

The xfce4-appfinder is a program that searches your file system for .desktop files, and displays a categorized list of all the GUI applications on your system.


## Installation

The file [`INSTALL`](INSTALL) contains generic installation instructions.


## Debugging Support

xfce4-appfinder currently supports three different levels of debugging support,
which can be setup using the configure flag `--enable-debug` (check the output
of `configure --help`):

| Argument  | Description |
| -------   | ----------- |
|  `yes`    | This is the default for Git snapshot builds. It adds all kinds of checks to the code, and is therefore likely to run slower. Use this for development of xfce4-appfinder and locating bugs in xfce4-appfinder. |
| `minimum` | This is the default for release builds. **This is the recommended behaviour.** |
| `no`      | Disables all sanity checks. Don't use this unless you know exactly what you do. |

## How to report bugs?

Bugs should be reported to the [Xfce bug tracking system](https://bugzilla.xfce.org)
against the product xfce4-appfinder. You will need to create an account for yourself.
