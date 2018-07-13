#!/bin/bash

gcc src/grant.c -O2 -Wall -o bin/grant -pthread -lm -lgsl -lgslcblas -lexpat
