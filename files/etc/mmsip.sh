#!/bin/sh
#sleep 20                #休眠20秒后执行以下命令
#开机启动修改ip
#添加开机启动
#chmod 777 /etc/mmsip.sh
#sh /etc/mmsip.sh

LAN_IP=$(uci get network.lan.ipaddr)
sed -i s/192\\.168\\.[0-9]*\\.[0-9]*/$LAN_IP/g /www/mms.html
LAN_IP=$(uci get network.lan.ipaddr)
sed -i s/192\\.168\\.[0-9]*\\.[0-9]*/$LAN_IP/g /usr/lib/lua/luci/view/mms/mms.htm

LAN_IP=$(uci get network.lan.ipaddr)
sed -i s/192\\.168\\.[0-9]*\\.[0-9]*/$LAN_IP/g /www/mpd.html
LAN_IP=$(uci get network.lan.ipaddr)
sed -i s/192\\.168\\.[0-9]*\\.[0-9]*/$LAN_IP/g /usr/lib/lua/luci/view/mms/mpd.html

exit 0