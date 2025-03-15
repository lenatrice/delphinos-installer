#!/bin/bash
docker run -it --rm -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -v /run/dbus/system_bus_socket:/run/dbus/system_bus_socket \
    -v /mnt/new_root:/mnt/new_root \
    -v /mnt/new_root/boot:/mnt/new_root/boot \
    -v /home/lena/Projects/delphinos-installer:/app \
    --privileged \
    delphinos-installer:latest bash -c "/app/build/delphinos-installer & exec bash"