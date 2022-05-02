pkill -9 -f "novnc" && sudo pkill -9 x11vnc && pkill -9 -f "xf" && sudo pkill -9 Xorg
sudo rm -f /tmp/.X1-lock
sudo X ${DISPLAY} -config /etc/X11/xorg.conf > /dev/null 2>&1 & disown
startxfce4 > /dev/null 2>&1 & disown
sudo x11vnc -localhost -display ${DISPLAY} -N -forever -shared > /dev/null 2>&1 & disown
/opt/novnc/utils/novnc_proxy --web /opt/novnc --vnc localhost:5901 --listen 6080 > /dev/null 2>&1 & disown
