FROM ubuntu:latest
LABEL org.opencontainers.image.title="iCub Gazebo Grasping Sandbox"
LABEL org.opencontainers.image.description="Infrastructure for running grasping experiments with the iCub humanoid in Gazebo"
LABEL org.opencontainers.image.source="https://github.com/robotology/icub-gazebo-grasping-sandbox"
LABEL org.opencontainers.image.authors="Ugo Pattacini <ugo.pattacini@iit.it>"

# Non-interactive installation mode
ENV DEBIAN_FRONTEND=noninteractive

# Update apt database
RUN apt update

# Install essentials
RUN apt install -y apt-utils software-properties-common apt-transport-https sudo \
    psmisc tmux nano wget curl telnet gnupg gdb git gitk autoconf locales gdebi \
    terminator meld dos2unix meshlab

# Set the locale
RUN locale-gen en_US.UTF-8

# Install graphics
RUN apt install -y xfce4 xfce4-goodies xserver-xorg-video-dummy xserver-xorg-legacy x11vnc && \
    apt remove -y xfce4-power-manager xfce4-screensaver light-locker && \
    apt autoremove -y && \
    sed -i 's/allowed_users=console/allowed_users=anybody/' /etc/X11/Xwrapper.config
COPY xorg.conf /etc/X11/xorg.conf
RUN dos2unix /etc/X11/xorg.conf

# Install browser
RUN wget https://dl.google.com/linux/direct/google-chrome-stable_current_amd64.deb && \
    gdebi -n google-chrome-stable_current_amd64.deb && \
    rm google-chrome-stable_current_amd64.deb

# Install python
RUN apt install -y python3 python3-dev python3-pip python3-setuptools && \
    if [ ! -f "/usr/bin/python" ]; then ln -s /usr/bin/python3 /usr/bin/python; fi
    
# Install magic-wormwhole to get things from one computer to another safely
RUN apt install -y magic-wormhole

# Install glog
RUN apt install -y libgoogle-glog-dev

# Install noVNC
RUN git clone https://github.com/novnc/noVNC.git /opt/novnc && \
    git clone https://github.com/novnc/websockify /opt/novnc/utils/websockify && \
    echo "<html><head><meta http-equiv=\"Refresh\" content=\"0; url=vnc.html?autoconnect=true&reconnect=true&reconnect_delay=1000&resize=scale&quality=9\"></head></html>" > /opt/novnc/index.html

# Select options
ARG ROBOTOLOGY_SUPERBUILD_RELEASE
ARG BUILD_TYPE
ARG ROBOTOLOGY_SUPERBUILD_INSTALL_DIR=/robotology-superbuild-install

# Set up git (required by superbuild)
RUN git config --global user.name "GitHub Actions" && \
    git config --global user.email "actions@github.com"
    
# Install dependencies
RUN git clone https://github.com/robotology/robotology-superbuild.git --depth 1 --branch ${ROBOTOLOGY_SUPERBUILD_RELEASE} && \
    robotology-superbuild/scripts/install_apt_dependencies.sh
    
RUN sh -c 'echo "deb http://packages.osrfoundation.org/gazebo/ubuntu-stable `lsb_release -cs` main" > /etc/apt/sources.list.d/gazebo-stable.list' && \
    wget https://packages.osrfoundation.org/gazebo.key -O - | tee /etc/apt/trusted.gpg.d/gazebo.asc && \
    apt update && \
    apt install -y libpcl-dev libcgal-dev gazebo libgazebo-dev

# Build robotology-superbuild
RUN cd robotology-superbuild && mkdir build && cd build && \
    cmake .. \
          -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
          -DYCM_EP_INSTALL_DIR=${ROBOTOLOGY_SUPERBUILD_INSTALL_DIR} \
          -DROBOTOLOGY_ENABLE_CORE:BOOL=ON \
          -DROBOTOLOGY_ENABLE_GRASPING:BOOL=ON \
          -DROBOTOLOGY_USES_GAZEBO:BOOL=ON \
          -DROBOTOLOGY_USES_PCL_AND_VTK:BOOL=ON && \
    make && \
    cd ../.. && rm -Rf robotology-superbuild

# Clean up git configuration
RUN git config --global --unset-all user.name && \
    git config --global --unset-all user.email

# Set environmental variables
ENV DISPLAY=:1

# Create user gitpod
RUN useradd -l -u 33333 -G sudo -md /home/gitpod -s /bin/bash -p gitpod gitpod && \
    # passwordless sudo for users in the 'sudo' group
    sed -i.bkp -e 's/%sudo\s\+ALL=(ALL\(:ALL\)\?)\s\+ALL/%sudo ALL=NOPASSWD:ALL/g' /etc/sudoers

# Switch to gitpod user
USER gitpod

# Install informative git for bash
RUN git clone https://github.com/magicmonty/bash-git-prompt.git ~/.bash-git-prompt --depth=1

# Set up .bashrc
WORKDIR /home/gitpod
RUN echo "GIT_PROMPT_ONLY_IN_REPO=1" >> ~/.bashrc && \
    echo "source \${HOME}/.bash-git-prompt/gitprompt.sh" >> ~/.bashrc && \
    echo "YARP_COLORED_OUTPUT=1" >> ~/.bashrc && \
    echo "source ${ROBOTOLOGY_SUPERBUILD_INSTALL_DIR}/share/robotology-superbuild/setup.sh" >> ~/.bashrc && \
    echo "LD_LIBRARY_PATH=/usr/local/lib:\${LD_LIBRARY_PATH}" >>  ~/.bashrc

# Create the Desktop dir
RUN mkdir -p /home/gitpod/Desktop

# Switch back to root
USER root

# Set up script to launch graphics and vnc
COPY start-vnc-session.sh /usr/bin/start-vnc-session.sh
RUN chmod +x /usr/bin/start-vnc-session.sh && \
    dos2unix /usr/bin/start-vnc-session.sh

# Make sure specific dirs are owned by gitpod user
RUN chown -R gitpod.gitpod ${ROBOTOLOGY_SUPERBUILD_INSTALL_DIR}

# Manage ports
EXPOSE 5901 6080 10000/tcp 10000/udp

# Workaround for mesa-utils (https://launchpad.net/~kisak/+archive/ubuntu/kisak-mesa)
RUN add-apt-repository -y ppa:kisak/kisak-mesa && \
    apt update && \
    apt install -y aptitude && \
    aptitude full-upgrade -y

# Clean up unnecessary installation products
RUN rm -Rf /var/lib/apt/lists/*

# Launch bash from /workspace
WORKDIR /workspace
CMD ["bash"]
