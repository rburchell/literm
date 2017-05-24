#!/bin/sh

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    sudo add-apt-repository ppa:beineri/opt-qt58-trusty -y
    sudo apt-get update -qq
fi
