#!/bin/bash
#
make clean && make -C ibrcommon clean
sudo make uninstall
sudo make -C ibrcommon uninstall

