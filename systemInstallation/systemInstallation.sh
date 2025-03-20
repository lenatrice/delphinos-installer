#!/bin/bash

# Handle interrupts and clean up
script_dir=$(dirname "$(realpath "$0")")
source $script_dir/common;

echo "Script dir: $script_dir"

trap 'echo "Script interrupted. Cleaning up..."; chroot_teardown; exit 1' SIGINT SIGTERM EXIT ERR 

packages=("$@")

newroot="/mnt/new_root"

installationProcedureList=(
  "PREPARE NEW ROOT:",
  "INSTALLING:base:",
  "INSTALLING:grub:"
)

for pkg in "${packages[@]}"; do
    installationProcedureList+=("INSTALLING:$pkg:")
done

if [ -d /sys/firmware/efi ]; then
  installationProcedureList+=("INSTALLING:UEFI bootloader:", "CONFIGURING:UEFI bootloader:")
else
  installationProcedureList+=("INSTALLING:BIOS bootloader:", "CONFIGURING:BIOS bootloader:")
fi

installationProcedureList+=(
  "ACTIVATING:NetworkManager:",
  "ACTIVATING:iwd:",
  "ACTIVATING:sddm:",
  "GENERATING:fstab:"
)

procedureCount=${#installationProcedureList[@]}
echo "PROCEDURECOUNT:$procedureCount:"

setInstallationProgress() {
  echo "$1"
  for i in "${!installationProcedureList[@]}"; do
      procedure="${installationProcedureList[i]}"
      procedure=$(echo "$procedure" | sed 's/,$//')  # Remove trailing comma
      if [[ "$procedure" == "$1" ]]; then
          installationProgress=$i
          echo "PROGRESS:$installationProgress:"
          return;
      fi
  done
  echo "Warning: '$1' not found in the procedure list"
}

export -f setInstallationProgress

if [ ! -d "$newroot" ]; then
    echo "ERROR:$newroot is not a directory:"
    exit 1
fi

if ! findmnt $newroot > /dev/null; then
    echo "Error:$newroot is not a mountpoint for a partition:"
    exit 2;
fi

setInstallationProgress "PREPARE NEW ROOT:"

# Ensure required directories exist
echo "Creating directories in $newroot..."
mkdir -m 0755 -p "$newroot"/var/{cache/pacman/pkg,lib/pacman,log} "$newroot"/{dev,run,etc/pacman.d}
mkdir -m 1777 -p "$newroot"/tmp
mkdir -m 0555 -p "$newroot"/{sys,proc}

echo "Copying pacman gpupg and mirrorlist"

if [ -d /etc/pacman.d/gnupg ]; then
  cp -a /etc/pacman.d/gnupg "$newroot/etc/pacman.d/"
fi

if [ -f /etc/pacman.d/mirrorlist ]; then
  cp /etc/pacman.d/mirrorlist "$newroot/etc/pacman.d/"
fi

echo "Copying installation scripts to new root"
mkdir -p $newroot/systemInstallation
cp -v -r "$script_dir" "$newroot"

echo "Running chroot setup"
chroot_setup $newroot

packages_str="${packages[*]}"

export DBPATH=/var/lib/pacman/

sudo rm -f "/mnt/new_root/var/lib/pacman/db.lck"
setInstallationProgress "INSTALLING:base:"
pacman --noconfirm --root $newroot -Sy base

setInstallationProgress "INSTALLING:grub:"
pacman --noconfirm --root $newroot -Sy grub

echo "Chrooting on $newroot and running /systemInstallation/installPackages.sh $packages_str"

installationProcedureListStr=$(IFS=,; echo "${installationProcedureList[*]}")

chroot $newroot env \
  installationProcedureList="$installationProcedureListStr" \
  installationProgress="$installationProgress" \
  /systemInstallation/installPackages.sh $packages_str

chroot_teardown

cp -v -r $newroot/systemInstallation/systemFiles/* $newroot

rm -r $newroot/systemInstallation

setInstallationProgress "GENERATING:fstab:"

genfstab $newroot > $newroot/etc/fstab

if [ $? -ne 0 ]; then
  echo "ERROR:Could not generate fstab file:"
  exit 3
fi

echo "Successfully generated fstab file for $newroot"