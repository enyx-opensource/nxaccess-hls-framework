#! /bin/bash

pylint3 \
    --output-format=text \
    $(find -type f -name "*.py" ! -path "**/.venv/**")
