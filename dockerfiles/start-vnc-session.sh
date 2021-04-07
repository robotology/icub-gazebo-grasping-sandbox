pkill -9 -f "vnc" && pkill -9 -f "xf" && sudo pkill -9 Xorg
sudo rm -f /tmp/.X1-lock
sudo nohup X ${DISPLAY} -config /etc/X11/xorg.conf > /dev/null 2>&1 &
nohup startxfce4 > /dev/null 2>&1 &
nohup x11vnc -localhost -display ${DISPLAY} -N -forever -shared -bg > /dev/null 2>&1
nohup /opt/novnc/utils/launch.sh --web /opt/novnc --vnc localhost:5901 --listen 6080 > /dev/null 2>&1 &
