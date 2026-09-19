# !/bin/bash

sudo dd if=myos.iso \
         of=/dev/disk4 \
         bs=4M \
         status=progress \
         conv=fsync

sync