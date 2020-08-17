
Sandbox to Simulate Grasping in Gazebo with the iCub
====================================================

[![ZenHub](https://img.shields.io/badge/Shipping_faster_with-ZenHub-435198.svg)](https://zenhub.com)
[![Join the community on Spectrum](https://withspectrum.github.io/badge/badge.svg)](https://spectrum.chat/icub)

## [📚 Documentation](https://robotology.github.io/icub-gazebo-grasping-sandbox)

## ☁ Instructions to run the sandbox on the web
We make use of the [Gitpod Cloud IDE](https://gitpod.io) as infrastructure. Find out more on [YARP-enabled Gitpod workspaces][1].

Here's below a quick how-to guide; please, refer instead to [📚 Documentation](#-documentation) for a complete video tutorial 🎥

1. To get started with the sandbox, click on the following badge:

    [![Gitpod](https://gitpod.io/button/open-in-gitpod.svg)][2]

1. Once the sandbox workspace is ready, build and install the project:
    ```sh
    $ cd /workspace/icub-gazebo-grasping-sandbox 
    $ mkdir build && cd build
    $ cmake ../
    $ make install
    ```
1. From within Gitpod, open up the browser at the port `6080` to get to the workspace desktop GUI.
1. In the desktop GUI, open a terminal and run the grasping experiment:
   ```sh
   $ icub-grasp.sh
   ```

<p align="center">
    <img src="./assets/showcase.gif">
</p>

## [🔽 Instructions to run the sandbox locally](./dockerfiles)

## 👨🏻‍💼 Maintainers
This repository is maintained by:

| | |
|:---:|:---:|
| [<img src="https://github.com/pattacini.png" width="40">](https://github.com/pattacini) | [@pattacini](https://github.com/pattacini) |

### 🙏 Acknowledgements
Special thanks go to [xEnVrE](https://github.com/xEnVrE) for his help on simulating visuomanipulation tasks in Gazebo.

### 🆕 Contributing
Check out our [CONTRIBUTING guidelines](./.github/CONTRIBUTING.md).

[1]: https://spectrum.chat/icub/technicalities/yarp-enabled-gitpod-workspaces-available~73ab5ee9-830e-4b7f-9e99-195295bb5e34
[2]: https://gitpod.io/#https://github.com/robotology/icub-gazebo-grasping-sandbox
