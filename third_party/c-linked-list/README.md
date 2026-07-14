# Header-only Linked List in C

This is a header-only linked list library written in C.

## Table of Contents

1. [About the Project](#about-the-project)
2. [Project Status](#project-status)
3. [Getting Started](#getting-started)
    1. [Requirements](#requirements)
    2. [Getting the Source](#getting-the-source)
    3. [Building](#building)
4. [Configuration Options](#configuration-options)
5. [Documentation](#documentation)
6. [Need Help?](#need-help)
7. [Contributing](#contributing)
8. [Further Reading](#further-reading)
9. [Authors](#authors)
10. [License](#license)
11. [Acknowledgments](#acknowledgements)

# About the Project

This is a header-only linked list library implemented in C. This code has been carried with me and improved over the years. With some research it seems that the implementation that passed into my hands was based on the Linux Kernel's implementation.

You can copy this header to your own projects, or you can consume this repository as a Meson subproject. With Meson, the header can be added to the relevant build components with the `c_linked_list_dep` variable.

To use this library, embed an `ll_t` element in a structure:

```c
typedef struct
{
    ll_t node;
    size_t size;
    char* block;
} alloc_node_t;
```

You then need to declare a linked list variable using the `LIST_INIT` macro:

```c
LIST_INIT(free_list);
```

Operations on the list primarily operate through your `ll_t` `struct` element.

To add a new element to the list, you can use `list_add` (or `list_add_tail`, or `list_insert`) to add the `ll_t` element to the desired list variable.

```c
// Note the pointer to the node element!
list_add(&new_memory_block->node, &free_list);
```

Removing an element only requires the `node` variable:

```c
list_del(&found_block->node);
```

Functions for iterating over the list are also provided:

```c
// Declare a variable to hold a pointer to the current element
// in the processing loop
alloc_node_t* current_block = NULL;

// Iterate over each element in the list
// First param is a variable in your struct type
// Second param is the list to iterate over
// Third param is the ll_t element in your struct type.
list_for_each_entry(current_block, &free_list, node)
{
    // perform an operation on current_block
}
```

Full documentation and a complete list of available functions can be found in the `ll.h` file.

For a full example of this library in action, see [embeddedartistry/libmemory](https://github.com/embeddedartistry/libmemory) and the ["freelist" implementation](https://github.com/embeddedartistry/libmemory/blob/master/src/malloc_freelist.c).

**[Back to top](#table-of-contents)**

# Project Status

This header implementation has been constant for years.

Since the header is now in a standalone repository, I would like to add test cases.

**[Back to top](#table-of-contents)**

## Getting Started

### Requirements

At a minimum you will need:

* `git-lfs`, which is used to store binary files in this repository
* Meson is the build system
* Some kind of compiler for your target system.
    - This repository has been tested with:
        - gcc-7, gcc-8, gcc-9
        - arm-none-eabi-gcc
        - Apple clang
        - Mainline clang

### Getting the Source

This project is hosted on [GitHub](https://github.com/embeddedartistry/c-linked-list). You can clone the project directly using this command:

```
git clone --recursive https://github.com/embeddedartistry/c-linked-list
```

### Building

If Make is installed, the library can be built by issuing the following command:

```
make
```

You can clean builds using:

```
make clean
```

You can eliminate the generated `buildresults` folder using:

```
make distclean
```

You can also use `meson` directly for compiling.

Create a build output folder:

```
meson buildresults
```

And build all targets by running

```
ninja -C buildresults
```

## Configuration Options

The following meson project options can be set for this library when creating the build results directory with `meson`, or by using `meson configure`:

* `disable-builtins` will tell the compiler not to generate built-in function
* `disable-stack-protection` will tell the compiler not to insert stack protection calls
* `enable-pedantic`: Turn on `pedantic` warnings
* `enable-pedantic-error`: Turn on `pedantic` warnings and errors

Options can be specified using `-D` and the option name:

```
meson buildresults -Ddisable-builtins=false
```

The same style works with `meson configure`:

```
cd buildresults
meson configure -Ddisable-builtins=false
```

**[Back to top](#table-of-contents)**

## Documentation

Documentation can be built locally by running the following command:

```
make docs
```

Documentation can be found in `buildresults/docs`, and the root page is `index.html`.

**[Back to top](#table-of-contents)**

## Need help?

If you need further assistance or have any questions, please file a GitHub issue or send us an email using the [Embedded Artistry Contact Form](http://embeddedartistry.com/contact).

You can also [reach out on Twitter: mbeddedartistry](https://twitter.com/mbeddedartistry/).

## Contributing

If you are interested in contributing to this project, please read our contributing guidelines.

## Authors

* **[Phillip Johnston](https://github.com/phillipjohnston)**

## License

Copyright (C) 2022 Embedded Artistry LLC

See the [LICENSE](LICENSE) file for licensing details.

**[Back to top](#table-of-contents)**
