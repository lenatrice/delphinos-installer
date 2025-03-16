#!/bin/bash
source /systemInstallation/common

packages=("$@")
total=${#packages[@]}
counter=0

echo "Running installation of packages"

# Synchronize the package databases
pacman --noconfirm -Sy

for pkg in "${packages[@]}"; do
    counter=$((counter + 1))
    setCurrentProgress "INSTALLING:$pkg:"
    pacman --noconfirm -S "$pkg"
done

echo "Installation finished successfully"

# Detect if system is UEFI or BIOS and install the bootloader accordingly
if [ -d /sys/firmware/efi ]; then
    setCurrentProgress "INSTALLING:UEFI bootloader:"
    grub-install --target=x86_64-efi --efi=/boot && echo "Successfully installed UEFI bootloader"
    if [ $? -ne 0 ]; then
        echo "ERROR:could not install UEFI bootloader"
        exit 2
    fi
    setCurrentProgress "CONFIGURING:UEFI bootloader:"
    grub-mkconfig -o /boot/grub/grub.cfg
    if [ $? -ne 0 ]; then
        echo "ERROR:Could not generate UEFI bootloader configuration:"
        exit 3
    fi
else
    # Detect the device mounted on /boot for BIOS systems
    boot_device=$(findmnt -n -o SOURCE /boot)
    
    if [ -z "$boot_device" ]; then
        echo "ERROR:Could not detect the device mounted on /boot:"
        exit 1
    fi

    # Install BIOS bootloader on the detected device
    setCurrentProgress "INSTALLING:BIOS bootloader:"
    grub-install --target=i386-pc "$boot_device" && echo "Successfully installed BIOS bootloader"
    if [ $? -ne 0 ]; then
        echo "ERROR:Could not generate BIOS bootloader configuration:"
        exit 2
    fi
    setCurrentProgress "CONFIGURING:BIOS bootloader:"
    grub-mkconfig -o /boot/grub/grub.cfg
        if [ $? -ne 0 ]; then
        echo "ERROR:generating BIOS bootloader configuration:"
        exit 3
    fi
fi

chpasswd <<< "root:root"

setCurrentProgress "ACTIVATING:NetworkManager:"
systemctl enable NetworkManager
setCurrentProgress "ACTIVATING:iwd:"
systemctl enable iwd
setCurrentProgress "ACTIVATING:sddm:"
systemctl enable sddm


exit 0