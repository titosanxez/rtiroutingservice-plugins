# RTI Routing Service Forwarding Engine Plugin

This repository contains multiple Processor Plugins for RTI Routing Service which can be
used to propagate data within a Route, from Inputs to Outputs, based on different criteria.

The library built by the repository (`rtirsfwdprocessor`) provides two plugins:

  - **ByInputNameForwardingEnginePlugin**

    - Forwards samples based on the name of the Input that produced them.

    - Created by registering function
     `RTI_PRCS_FWD_ByInputNameForwardingEnginePlugin_create`.

 - **ByInputValueForwardingEnginePlugin**

   - Forwards samples based on the contents of one of their members.

   - Created by registering function
    `RTI_PRCS_FWD_ByInputValueForwardingEnginePlugin_create`


## Cloning

This repository uses the simple build system provided by [rtiroutingservice-plugin-helper](https://bitbucket.rti.com/users/asorbini/repos/rtiroutingservice-plugin-helper/). This repository must be cloned separately.

The helper repository also uses git submodules, which can be automatically
initialized during cloning using the `--recurse-submodules` option.

To clone this repository, along with all required dependencies:

```sh
git clone https://bitbucket.rti.com/scm/~asorbini/rtiroutingservice-process-fwd.git
git clone --recurse-submodules https://bitbucket.rti.com/scm/~asorbini/rtiroutingservice-plugin-helper.git
```

## Building

After cloning all required repositories, the project can be built using [CMake](https://cmake.org/download/).

At a minimum, you must specify the following variable:

| Variable    | Values         | Description    |
|:-----------:|:--------------:|:---------------|
| `CONNEXTDDS_DIR` | Directory path | Installation of RTI Connext DDS with Routing Service SDK |
| `CONNEXTDDS_ARCH` | RTI's architecture identifier | Target build platform |


If you are using `cmake` from the command line, you can pass these
variables using the `-D` option, or set them in the shell's environment.

If you are using `cmake-gui` (or any other CMake interface) make sure these values are
provided as environment variables.

To configure the build from the command line:

```sh
mkdir build
cd build
cmake /path/to/rtiroutingservice-process-cbr \
      -DCONNEXTDDS_DIR=/path/to/rti_connext_dds \
      -DCONNEXTDDS_ARCH=targetCompilerAndOs \
      -DRSHELPER_DIR=/path/to/rtiroutingservice-plugin-helper 
```

Depending on the CMake generator that you used, you can let `cmake` perform the build
process (by invoking the native build tool) using the `--build` option:

```sh
cd build
cmake --build .
```

### Build Options

The following table summarizes all available CMake build options
(in addition to the required ones):

| Variable                |   Description                       | Default |
|:-----------------------:|:-----------------------------------:|:--------|
| DISABLE_LOG             | Disable all output of log messages to stdout | `OFF`  |
| ENABLE_DOCS             | Build included documentation. Requires [Doxygen](http://www.doxygen.nl/), [Sphinx](http://www.sphinx-doc.org/en/master/), and [Breathe](https://breathe.readthedocs.io/en/latest/) to be installed. | `OFF`  |
| ENABLE_EXAMPLES         | Build example applications          | `ON`  |
| ENABLE_LOG              | Force output of log messages to stdout. Enabled automatically for Debug builds, unless `DISABLE_LOG` is used | `OFF`  |
| ENABLE_TESTS            | Build tests                         | `ON`  |
| ENABLE_TRACE            | Enable trace level logging          | `OFF`  |

### Installation

The "install" step can be run to copy all files generated by the build 
process into a single location specified by `CMAKE_PREFIX_INSTALL`.

For the "Unix Makefiles" and "Ninja" CMAke generators, this step can be
automatically run by performing the build using the `install` target:

```sh
mkdir build
cd build
cmake /path/to/rtiroutingservice-process-cbr \
      -DCONNEXTDDS_DIR=/path/to/rti_connext_dds \
      -DCONNEXTDDS_ARCH=targetCompilerAndOs \
      -DRSHELPER_DIR=/path/to/rtiroutingservice-plugin-helper \
      -DRTI_TSFM_DIR=/path/to/rtiroutingservice-transform
cmake --build . -- install
```
