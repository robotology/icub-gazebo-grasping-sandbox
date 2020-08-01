X :1 -config /etc/X11/xorg.conf &
startxfce4 &
x11vnc -display :1 -N -forever -shared &
/opt/novnc/utils/launch.sh --web /opt/novnc --vnc 0.0.0.0:5901 --listen 6080 &
