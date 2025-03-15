#!/bin/bash

# Handle interrupts and clean up
trap 'echo "Script interrupted. Cleaning up..."; chroot_teardown; exit 1' SIGINT SIGTERM ERR

packages=("$@")

if [ ${#packages[@]} -eq 0 ]; then
    echo "No packages provided."
    exit 1
fi

newroot=/mnt/new_root
total=${#packages[@]}
counter=0

if [ $# -eq 0 ]; then
  set -- base
fi

if [ ! -d "$newroot" ]; then
  echo "Error: $newroot is not a directory."
  exit 1
fi

# Ensure required directories exist
echo "Creating directories in $newroot..."
mkdir -m 0755 -p "$newroot"/var/{cache/pacman/pkg,lib/pacman,log} "$newroot"/{dev,run,etc/pacman.d}
mkdir -m 1777 -p "$newroot"/tmp
mkdir -m 0555 -p "$newroot"/{sys,proc}

# Set up chroot environment

ignore_error() {
  "$@" 2>/dev/null
  return 0
}

chroot_add_mount() {
  mount "$@" && CHROOT_ACTIVE_MOUNTS=("$2" "${CHROOT_ACTIVE_MOUNTS[@]}")
}

chroot_maybe_add_mount() {
  local cond=$1; shift
  if eval "$cond"; then
    chroot_add_mount "$@"
  fi
}

chroot_setup() {
  CHROOT_ACTIVE_MOUNTS=()
  [[ $(trap -p EXIT) ]] && die '(BUG): attempting to overwrite existing EXIT trap'
  trap 'chroot_teardown' EXIT

  chroot_add_mount proc "$1/proc" -t proc -o nosuid,noexec,nodev &&
  chroot_add_mount sys "$1/sys" -t sysfs -o nosuid,noexec,nodev,ro &&
  ignore_error chroot_maybe_add_mount "[[ -d '$1/sys/firmware/efi/efivars' ]]" \
      efivarfs "$1/sys/firmware/efi/efivars" -t efivarfs -o nosuid,noexec,nodev &&
  chroot_add_mount udev "$1/dev" -t devtmpfs -o mode=0755,nosuid &&
  chroot_add_mount devpts "$1/dev/pts" -t devpts -o mode=0620,gid=5,nosuid,noexec &&
  chroot_add_mount shm "$1/dev/shm" -t tmpfs -o mode=1777,nosuid,nodev &&
  chroot_add_mount /run "$1/run" --bind --make-private &&
  chroot_add_mount tmp "$1/tmp" -t tmpfs -o mode=1777,strictatime,nodev,nosuid
}

# Teardown chroot environment
chroot_teardown() {
  if (( ${#CHROOT_ACTIVE_MOUNTS[@]} )); then
    umount "${CHROOT_ACTIVE_MOUNTS[@]}"
  fi
  unset CHROOT_ACTIVE_MOUNTS
}

chroot_setup $new_root

if [ -d /etc/pacman.d/gnupg ]; then
  cp -a /etc/pacman.d/gnupg "$newroot/etc/pacman.d/"
fi

if [ -f /etc/pacman.d/mirrorlist ]; then
  cp /etc/pacman.d/mirrorlist "$newroot/etc/pacman.d/"
fi

pacman --noconfirm --root "$newroot" --config /etc/pacman.conf -Sy

for pkg in "${packages[@]}"; do
    counter=$((counter + 1))
    echo "INSTALLING: $pkg ($counter/$total)"
    pacman --noconfirm --root "$newroot" --config /etc/pacman.conf -S "$pkg"
done

chroot_teardown

echo "Installation finished successfully on $newroot"

#test