icub-client: A coherent framework for complex HRI scenarios on the iCub
=======

Generating a complex, human-like behaviour in a humanoid robot such as the iCub requires the integration of a wide range of open source components and a scalable cognitive architecture. Hence, we present the `icub-client` library which provides convenience wrappers for components related to perception (object recognition, agent tracking, speech recognition, touch detection), object manipulation (various basic and complex motor actions), and social interaction (speech synthesis, joint attention). Communication between those components is orchestrated by a working memory, and a long-term memory allows the iCub to re-visit and reason about earlier interactions. In addition to previously integrated components, the library allows for simple extension to new components, and rapid prototyping by adapting to changes in interfaces between components. We also provide a set of modules which make use of the library, such as a high-level planner and an action recognition module. The proposed architecture has been successfully employed for a complex human robot interaction scenario involving the acquisition of language capabilities, execution of goal-oriented behaviour and expression of a verbal narrative of the robot’s experience in the world. The architecture is aimed at researchers familiarising themselves with the iCub ecosystem, as well as expert users.

## Documentation
Visit the official Project [wiki](http://robotology.github.com/icub-client) for the software.

## License
The `icub-client` library and documentation are distributed under the GPL.
The full text of the license agreement can be found in: [./LICENSE](https://github.com/robotology/icub-client/blob/master/LICENSE).

Please read this license carefully before using the `icub-client` code.

## CI Build
- Linux: [![Build Status](https://travis-ci.org/robotology/wysiwyd.png?branch=master)](https://travis-ci.org/robotology/wysiwyd)
- Windows: [![Build status](https://ci.appveyor.com/api/projects/status/4rckcp8suov8pcv1)](https://ci.appveyor.com/project/pattacini/wysiwyd)

## Build dependencies
`icub-client` depends on the following projects which need to be installed prior to building `icub-client:

### YARP, icub-main and icub-contrib-common
First, follow the [installation instructions](http://wiki.icub.org/wiki/Linux:Installation_from_sources) for `yarp`, `icub-main` and `icub-contrib-common`.


### OpenCV-3.2.0
**`OpenCV-3.0.0`** or higher (**`OpenCV-3.2.0`** is recommended) is a required dependency to build the `iol2opc` module which is responsible for object tracking. More specifically, we need the new tracking features delivered with `OpenCV-3.2.0`:

1. Download `OpenCV-3.2.0`: `git clone https://github.com/Itseez/opencv.git`.
2. Checkout the correct branch: `git checkout 3.2.0`.
3. Download the external modules: `git clone https://github.com/Itseez/opencv_contrib.git`.
4. Checkout the correct branch: `git checkout 3.2.0`.
5. Configure `OpenCV` by filling in the cmake var **`OPENCV_EXTRA_MODULES_PATH`** with the path pointing to `opencv_contrib/modules` and then toggling on the var **`BUILD_opencv_tracking`**.
6. Compile `OpenCV`.

### kinect-wrapper
To detect the human skeleton, we employ the [`kinect-wrapper` library](https://github.com/robotology/kinect-wrapper.git). Please follow the installation instructions in the readme. It might be the case that you have also to build `kinect-wrapper` with the new `OpenCV-3.x.x` library. We have enabled the possibility to build only the client part of the `kinect-wrapper` (see [**updated instructions**](https://github.com/robotology/kinect-wrapper#cmaking-the-project)).


### iol
For the object tracking, we rely on the `iol` pipeline. Please follow the [installation instructions](https://github.com/robotology/iol). For `icub-client`, not the full list of dependencies is needed. Only install the following dependencies: `segmentation`, `Hierarchical Image Representation`, and `stereo-vision`.


### speech
This requires a Windows machine with the [Microsoft speech SDK](https://msdn.microsoft.com/en-us/library/hh361572(v=office.14).aspx) installed. Then, compile the [`speech` repository](https://github.com/robotology/speech) for speech recognition and speech synthesis.

## Build icub-client

Once all dependencies are installed, building `icub-client` is straightforward:

1. Download `icub-client`: `git clone https://github.com/robotology/icub-client.git`.
2. `cd icub-client`
3. `mkdir build`
4. `cd build`
5. `ccmake ..` and fill in the cmake var **`OpenCV_DIR`** with the path to the `OpenCV-3.2.0` build.
6. Compile `icub-client` using `make`.

### Update software script
We provide a [Python script](https://github.com/robotology/icub-client/blob/master/update-software.py) to easily update all dependencies of `icub-client` and `icub-client` itself.
