# Write a useful README about how this make differs from all the other makes
- Motivation for writing this make
    - Make is great, but not really suitable for large projects
        - Either recursive make, or one gigantic Makefile (both are bad)
    - Alternatives
        - Autotools is way too painful and uses recursive make
        - CMake is too fat and is written in C++
        - Meson is written in Python
        - All of the above require learning new languages
- Fundamental difference between this make, and all the others
    - Targets/Dependencies are treated as paths, not strings
    - Every directory is associated with a scope
    - There is a Tree of Scopes
    - Minor differences
        - Inference rules can be arbitrarily deep
        - No need for `.SUFFIXES:`
        - Macro assignment works slightly different
            - use `?=` if you want to allow overriding an assignment
- Explain how this make works
    - Data Structures
        - `scope`
        - `directory` and `gnu`
        - `file`
        - `rule`
        - `dep`
    - Algorithms
        - `parse_recursive()` (bottom-up)
        - `parse()` (top-down)
        - lazy includes
        - `expand()`
        - `build()` and `build_file()`
        - Inference rules
- List of Features
    - "Usual" Make Features
    - Scoped Include
    - Integration with "foreign" make directories (GNU, BSD, ...)
    - "Exporting" macros into child scopes
- Where this make could be useful
    - Large projects with many subdirectories
- Future Work


# Options
-q	        check if anythings needs to be built
-i          ignore failing commands
-e          read environment variables

# Print a warning, if a generated file is not placed into `${.OBJDIR}` when building out-of-tree

# Allow leaf-directories to not have a Makefile

# Special macros
## Optional: `${@F}`, `${@D}`, `${<F}`, `${<F}`, `${*F}`, `${*D}`, `${^F}`, `${^D}`
Filename or Directory portions of the variables.

# Special targets
## `.TOP:` to stop the upward recursion
This must be defined at the top of the file.

## `.PATH:`
Allow specifying additional `PATH` directories.

## Allow using spaces instead of tabs

# Future TODOs
## Check names for validity
Allowed charset for variable names: `[a-zA-Z_.]+`
Allowed charset for file names: `[a-zA-Z_.-]+`
## Include file and position in every error message
## Write a tutorial on how to correctly use this make
## Write a specification
## Create a freestanding version, written in Rust, so that other projects can use this make

# Known Bugs
## Unnamed bug #1
### Problem
`clean: ${.SUBDIRS:=/clean}`
is interpreted as macro definition:
`clean: ${.SUBDIRS:` = `/clean}`
### Workaround
```make
CLEAN = ${.SUBDIRS:=/clean}
`clean: ${CLEAN}`
```
## Crash #1
```make
a:
    echo a
b:
    echo b
a a: b
```
