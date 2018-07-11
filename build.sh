#!/bin/bash

gcc src/grant.c -O2 -o bin/grant -pthread -lm -lgsl -lgslcblas -lexpat
