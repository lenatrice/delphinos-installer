#!/bin/bash
mount -o rw /dev/sdb2 /mnt/new_root
mount -o rw --mkdir /dev/sdb1 /mnt/new_root/boot