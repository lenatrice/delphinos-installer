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

# Se nenhum pacote for especificado, utiliza o grupo 'base'
if [ $# -eq 0 ]; then
  set -- base
fi

# Verifica se o diretório existe
if [ ! -d "$newroot" ]; then
  echo "Erro: $newroot não é um diretório."
  exit 1
fi

# Cria a estrutura de diretórios necessários no novo root
echo "Creating directories in $newroot..."
mkdir -p "$newroot"/{dev,proc,sys,run,tmp,etc/pacman.d,var/cache/pacman/pkg,var/lib/pacman,log}

# Função simplificada para montar os sistemas de arquivos no novo root
chroot_setup() {
  echo "Montando proc..."
  mount -t proc proc "$newroot/proc" -o nosuid,noexec,nodev || { echo "Erro montando proc"; exit 1; }

  echo "Montando sysfs..."
  mount -t sysfs sysfs "$newroot/sys" -o nosuid,noexec,nodev,ro || { echo "Erro montando sys"; exit 1; }

  if [ -d "$newroot/sys/firmware/efi/efivars" ]; then
    echo "Montando efivarfs..."
    mount -t efivarfs efivarfs "$newroot/sys/firmware/efi/efivars" -o nosuid,noexec,nodev || echo "Aviso: Erro montando efivarfs"
  fi

  echo "Montando devtmpfs..."
  mount --bind /dev "$newroot/dev" || { echo "Erro montando /dev"; exit 1; }
  
  echo "Montando devpts..."
  mount -t devpts devpts "$newroot/dev/pts" -o mode=0620,gid=5,nosuid,noexec || { echo "Erro montando devpts"; exit 1; }

  echo "Montando dev/shm..."
  mount -t tmpfs tmpfs "$newroot/dev/shm" -o mode=1777,nosuid,nodev || { echo "Erro montando dev/shm"; exit 1; }

  echo "Montando /run..."
  mount --bind /run "$newroot/run" || { echo "Erro montando /run"; exit 1; }

  echo "Montando tmp..."
  mount -t tmpfs tmpfs "$newroot/tmp" -o mode=1777,strictatime,nodev,nosuid || { echo "Erro montando tmp"; exit 1; }
}

# Função para desmontar os sistemas de arquivos montados
chroot_teardown() {
  echo "Desmontando sistemas de arquivos..."
  umount -R "$newroot/proc" "$newroot/sys" "$newroot/run" "$newroot/tmp"
  umount -R --lazy "$newroot/dev"
}

# Monta os sistemas de arquivos necessários
chroot_setup

# Copia o keyring do pacman (se existir) para o novo root
if [ -d /etc/pacman.d/gnupg ]; then
  echo "Copiando keyring do pacman..."
  cp -a /etc/pacman.d/gnupg "$newroot/etc/pacman.d/"
fi

# Copia o mirrorlist para o novo root (se existir)
if [ -f /etc/pacman.d/mirrorlist ]; then
  echo "Copiando mirrorlist..."
  cp /etc/pacman.d/mirrorlist "$newroot/etc/pacman.d/"
fi

pacman --noconfirm --root "$newroot" --config /etc/pacman.conf -Sy

# Instala os pacotes no novo root
echo "Instalando pacotes em $newroot: $*"
for pkg in "${packages[@]}"; do
    counter=$((counter + 1))
    echo "INSTALLING:$pkg:$counter:$total"
    pacman --noconfirm --root "$newroot" --config /etc/pacman.conf -S "$pkg"
done

# Desmonta os sistemas de arquivos
chroot_teardown

echo "Instalação concluída com sucesso em $newroot"