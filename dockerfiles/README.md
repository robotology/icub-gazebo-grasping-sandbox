🐳 Instructions to run the sandbox locally
==========================================

To run the sandbox locally using [Docker](https://www.docker.com), go through the following steps:
1. Pull the docker image:
    ```sh
    $ docker pull pattacini/icub-gazebo-grasping-sandbox
    ```
1. Launch the container:
    ```sh
    $ docker run -it --rm -p 6080:6080 --user gitpod pattacini/icub-gazebo-grasping-sandbox
    ```
1. From within the container shell, launch the following scripts:
    ```sh
    $ init-icubcontrib-local.sh
    $ start-vnc-session-local.sh
    ```
1. Open up the browser and connect to **`localhost:6080`**.
1. Clone and install the project:
    ```sh
    $ git clone https://github.com/robotology/icub-gazebo-grasping-sandbox.git /workspace/icub-gazebo-grasping-sandbox
    $ cd /workspace/icub-gazebo-grasping-sandbox 
    $ mkdir build && cd build
    $ cmake ../
    $ make install
    ```
1. Run the grasping experiment:
   ```sh
   $ icub-grasp.sh
   ```
1. Once done, from the container shell press **CTRL+D**.
