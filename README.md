
Sandbox to Simulate Grasping in Gazebo with the iCub
====================================================

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.3999468.svg)](https://doi.org/10.5281/zenodo.3999468)
[![ZenHub](https://img.shields.io/badge/Shipping_faster_with-ZenHub-435198.svg)](https://zenhub.com)
[![Community](https://img.shields.io/badge/Join-Robotology_Community-blue?style=plastic&logo=github)](https://github.com/robotology/community)

## [🚶🏻‍♂️ Walkthrough][walkthrough]
This sandbox allows you to experiment with iCub performing basic object grasping within the Gazebo simulator.

## ☁ Instructions to run the sandbox on the web
Here's below a quick how-to guide. For a more detailed description of the grasping pipeline, please refer to the [**🚶🏻‍♂️ Walkthrough**][walkthrough].

1. To get started with the sandbox, click on the following badge:

    [![Open in GitHub Codespaces](https://github.com/codespaces/badge.svg)](https://codespaces.new/robotology/icub-gazebo-grasping-sandbox)

1. Once the sandbox workspace is ready, do the following:
    1. Click on the **PORTS** tab.
    2. Click on the browser icon 🌐 next to the nvc port `6080` to open the desktop view.
1. Within the terminal, build and install the project:
    ```console
    cmake -S . -B build
    cmake --build build/ --target install
    ```
1. Run the grasping experiment:
   ```console
   icub-grasp.sh
   ```
   You will see the robot in action in the desktop view.

Also:   
- You can run a systematic test with always the same object in the same pose by specifying the option `test`:
  ```console
  icub-grasp.sh test
  ```
  Afterward, inspect relevant quantities logged in `/tmp/icub-gazebo-grasping-sandbox.INFO`.
- If needed, you can clean up hanging resources by specifying the option `clean`:
  ```console
  icub-grasp.sh clean
  ```

<div align="center">
  <img src="./assets/showcase.gif">
</div>

## [🔽 Instructions to run the sandbox locally](./dockerfiles/README.md)

## ℹ Manual installation
If you want to install the sandbox manually to perform specific customization (e.g., enable GPU), please refer to the recipe contained in the [**`Dockerfile`**](./dockerfiles/Dockerfile).

## 📚 Cite the sandbox
In case you use this sandbox for your work, please **quote it within any resulting publication** by citing this [repository](./CITATION.cff) and/or the paper where you can find the **original implementation of the grasping method**:
- P. D. H. Nguyen, F. Bottarel, U. Pattacini, M. Hoffmann, L. Natale and G. Metta, "_Merging Physical and Social Interaction for Effective Human-Robot Collaboration_," 2018 IEEE-RAS 18th International Conference on Humanoid Robots (Humanoids), Beijing, China, 2018, pp. 1-9, doi: [10.1109/HUMANOIDS.2018.8625030](https://doi.org/10.1109/HUMANOIDS.2018.8625030).

<details>
<summary>🔘 Click to show other papers describing foundamental components used in the sandbox</summary>
<b>Motion control of iCub in the operational space:</b>
<ul>
  <li>U. Pattacini, F. Nori, L. Natale, G. Metta and G. Sandini, "<i>An experimental evaluation of a novel minimum-jerk cartesian controller for humanoid robots</i>," 2010 IEEE/RSJ International Conference on Intelligent Robots and Systems, Taipei, 2010, pp. 1668-1674, doi: <a href="https://doi.org/10.1109/IROS.2010.5650851">10.1109/IROS.2010.5650851</a>.</li>
  <li>A. Roncone, U. Pattacini, G. Metta and L. Natale, "<i>A Cartesian 6-DoF Gaze Controller for Humanoid Robots</i>", Proceedings of Robotics: Science and Systems, Ann Arbor, MI, June 18-22, 2016, doi: <a href="https://doi.org/10.15607/RSS.2016.XII.022">10.15607/RSS.2016.XII.022</a>.</li>
</ul>
<b>Interoperability between iCub and Gazebo:</b>
<ul>
  <li>M. Hoffman E., S. Traversaro, A. Rocchi, M. Ferrati, A. Settimi, F. Romano, L. Natale, A. Bicchi, F. Nori and N. G. Tsagarakis, "<i>Yarp Based Plugins for Gazebo Simulator</i>". In: Hodicky J. (eds) Modelling and Simulation for Autonomous Systems. MESAS 2014. Lecture Notes in Computer Science, vol 8906. Springer, Cham. pp 333-346, doi: <a href="https://doi.org/10.1007/978-3-319-13823-7_29">10.1007/978-3-319-13823-7_29</a>.
</ul>
</details>

## [📃 List of works built on the sandbox][list-works]

### 👨🏻‍💻 Maintainers
This repository is maintained by:

| | |
|:---:|:---:|
| [<img src="https://github.com/pattacini.png" width="40">](https://github.com/pattacini) | [@pattacini](https://github.com/pattacini) |

### 🙏 Acknowledgements
- This sandbox was made possible thanks to [xEnVrE](https://github.com/xEnVrE) of the [HSP@IIT][HSP] group for his contributions to [`robotology/icub-models`][icub-models] on simulating visuo-manipulation tasks in Gazebo.
- Inspired by the simplified simulation environment available in [`shadow-robot/smart_grasping_sandbox`][shadow-robot].

### 🆕 Contributing
Check out our [CONTRIBUTING guidelines](./.github/CONTRIBUTING.md).

[walkthrough]: https://robotology.github.io/icub-gazebo-grasping-sandbox
[list-works]: https://robotology.github.io/icub-gazebo-grasping-sandbox/building-on-sandbox.html
[HSP]: https://www.iit.it/research/lines/humanoid-sensing-and-perception
[icub-models]: https://github.com/robotology/icub-models
[shadow-robot]: https://github.com/shadow-robot/smart_grasping_sandbox
