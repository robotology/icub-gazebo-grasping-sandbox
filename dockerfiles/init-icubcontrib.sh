sudo chown -R gitpod.gitpod /workspace
mkdir -p /workspace/iCubContrib
git clone https://github.com/robotology/icub-contrib-common.git --depth 1 /workspace/icub-contrib-common
mkdir -p /workspace/icub-contrib-common/build
cd /workspace/icub-contrib-common/build
cmake .. -DCMAKE_INSTALL_PREFIX=/workspace/iCubContrib
make install
rm -Rf /workspace/icub-contrib-common
cd -
