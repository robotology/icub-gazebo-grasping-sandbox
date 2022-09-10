🔽 Instructions to run the sandbox locally
==========================================

To run the sandbox locally using [Docker](https://docs.docker.com/get-docker), go through the following steps:
1. Pull the docker image:
    ```console
    docker pull ghcr.io/robotology/icub-gazebo-grasping-sandbox:latest
    ```
1. Launch the container:
    ```console
    docker run -it --rm -p 6080:6080 --user gitpod ghcr.io/robotology/icub-gazebo-grasping-sandbox:latest
    ```
1. From within the container shell, launch the following scripts:
    ```console
    start-vnc-session.sh
    ```
1. Clone and install the project:
    ```console
    git clone https://github.com/robotology/icub-gazebo-grasping-sandbox.git
    cd icub-gazebo-grasping-sandbox
    mkdir build && cd build
    cmake ../
    make install
    ```
1. Open up the browser and connect to **`localhost:6080`** to get to the workspace desktop GUI.
1. In the desktop GUI, open a terminal and run the grasping experiment:
   ```console
   icub-grasp.sh
   ```
1. Once done, from the container shell press **CTRL+D**.

