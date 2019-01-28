#!/bin/bash

docker build --no-cache -t elixir-qtling-current \
       --build-arg GIT_REPO="https://github.com/edahlgren/QtLing" \
       .
