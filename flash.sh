#!/bin/bash

iso="$1"
disk="$2"

diskutil unmountDisk "$disk"

sudo dd if="$iso" \
    of="$disk" \
    bs=4M \
    status=progress \
    conv=fsync

sync