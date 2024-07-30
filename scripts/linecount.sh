#!/bin/bash

echo Linecount:
cat $(find tests/* src/* -type f) | wc -l
