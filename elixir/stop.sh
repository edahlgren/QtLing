#!/bin/bash

docker stop -t 1 $( docker ps | grep elixir-qtling-current | awk '{print $1}' )
