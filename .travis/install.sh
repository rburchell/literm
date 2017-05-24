#!/bin/sh

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    sudo apt-get -y install uuid-dev xvfb qt58base qt58declarative
fi
