#!/bin/bash

packages=("$@")

if [ ${#packages[@]} -eq 0 ]; then
    echo "No packages provided."
    exit 1
fi

newroot=/mnt/new_root
packages=("$@")
total=${#packages[@]}
counter=0

if [ $# -eq 0 ]; then
  set -- base
fi

if [ ! -d "$newroot" ]; then
  echo "Error: $newroot is not a directory."
  exit 1
fi

mkdir -p "$newroot"/{dev,proc,sys,run,tmp,etc/pacman.d,var/cache/pacman/pkg,var/lib/pacman,log}

chroot_setup() {
  mount -t proc proc "$newroot/proc" -o nosuid,noexec,nodev
  mount -t sysfs sysfs "$newroot/sys" -o nosuid,noexec,nodev,ro

  if [ -d "$newroot/sys/firmware/efi/efivars" ]; then
    mount -t efivarfs efivarfs "$newroot/sys/firmware/efi/efivars" -o nosuid,noexec,nodev || echo "Aviso: Erro montando efivarfs"
  fi

  mount --bind /dev "$newroot/dev"
  mount -t devpts devpts "$newroot/dev/pts" -o mode=0620,gid=5,nosuid,noexec
  mount -t tmpfs tmpfs "$newroot/dev/shm" -o mode=1777,nosuid,nodev
  mount --bind /run "$newroot/run"
  mount -t tmpfs tmpfs "$newroot/tmp" -o mode=1777,strictatime,nodev,nosuid
}

chroot_teardown() {
  umount -R "$newroot/proc" "$newroot/sys" "$newroot/run" "$newroot/tmp"
  umount -R --lazy "$newroot/dev"
}

chroot_setup

if [ -d /etc/pacman.d/gnupg ]; then
  cp -a /etc/pacman.d/gnupg "$newroot/etc/pacman.d/"
fi

if [ -f /etc/pacman.d/mirrorlist ]; then
  cp /etc/pacman.d/mirrorlist "$newroot/etc/pacman.d/"
fi

pacman --noconfirm --root "$newroot" --config /etc/pacman.conf -Sy

for pkg in "${packages[@]}"; do
    counter=$((counter + 1))
    echo "INSTALLING:$pkg:$counter:$total"
    pacman --noconfirm --root "$newroot" --config /etc/pacman.conf -S "$pkg"
done

chroot_teardown