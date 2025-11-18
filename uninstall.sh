#!/bin/bash
#
# Uninstall script.
# use chmod +x uninstall.sh
sudo xargs rm -fv < install_files.txt
exit
#rm -f ~/.cache/gnome-software/*
