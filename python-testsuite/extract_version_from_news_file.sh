#!/bin/bash
head -n 1 NEWS| perl -ne '$_ =~ /#\s*(.*)\s*/ ; print "$1\n"'