#!/bin/bash

export PYTHONPATH=.:$PYTHONPATH
python3 -m unittest discover -v -f --buffer --locals $@
