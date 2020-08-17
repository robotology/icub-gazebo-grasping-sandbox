pkill -9 -f "vnc" && pkill -9 -f "xf" && pkill -9 Xorg
rm -f /tmp/.X1-lock
nohup X :1 -config /etc/X11/xorg.conf > /dev/null 2>&1 &
nohup startxfce4 > /dev/null 2>&1 &
nohup x11vnc -display :1 -N -forever -loop -shared > /dev/null 2>&1 &
nohup /opt/novnc/utils/launch.sh --web /opt/novnc --vnc 0.0.0.0:5901 --listen 6080 > /dev/null 2>&1 &