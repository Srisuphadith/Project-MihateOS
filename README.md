# Project-MihateOS
Leaning how to develop OS, Moreover make new OS name MihateOS

---
<img src="./MihatePic.jpg">
---

Architechture x86 cossover on Mackbook pro M1 13"

### Install dependency
```
brew install \
    qemu \
    x86_64-elf-gcc \
    x86_64-elf-binutils \
    x86_64-elf-grub \
    i686-elf-grub \
    xorriso \
    nasm \
    mtools
```
### Build OS
```
make
```
### Clear build
```
make clean
```
### Run QEMU
```
make run
```