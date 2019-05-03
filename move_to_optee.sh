#!/bin/bash - 
cd ./build/product/optee/fw/release/obj

for i in `cat /media/vingu/vingu_ext/Linaro/Boards/FVP/SPCI_dev/SCP-firmware/build/product/optee/fw/obj-list.txt`; do ar x $i; done

cp /media/vingu/vingu_ext/Linaro/Boards/FVP/SPCI_dev/SCP-firmware/build/product/optee/fw/release/obj//media/vingu/vingu_ext/Linaro/Boards/FVP/SPCI_dev/SCP-firmware/build/product/optee/fw/fwk_module_list.o .

rm tmp.txt
for i in `ls *.o`; do printf "/media/vingu/vingu_ext/Linaro/Boards/FVP/SPCI_dev/SCP-firmware/build/product/optee/fw/release/obj/"$i" " >> tmp.txt; done

