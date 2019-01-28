#!/bin/bash

docker exec -i -t $( docker ps | grep elixir-qtling-current | awk '{print $1}' ) /bin/bash
