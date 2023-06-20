#!/bin/bash

echo "higher privilege might be require for a bunch of these commands"

service supervisor stop
cp catbot /opt/catbot/catbot
chown catbot /opt/catbot/catbot
chmod u+x /opt/catbot/catbot
setcap CAP_NET_BIND_SERVICE=+eip /opt/catbot/catbot
service supervisor start
