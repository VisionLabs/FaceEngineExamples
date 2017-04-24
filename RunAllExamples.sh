#!/bin/bash

echo ""

echo ""
echo "======================================="
echo "== RUNNING EXAMPLE 1 -- VERIFICATION =="
echo "======================================="
echo ""

./build/example1/Example1 images/Cameron_Diaz.ppm images/Cameron_Diaz_2.ppm 0.7

echo ""
echo "======================================="
echo "== RUNNING EXAMPLE 2 --  ATTRIBUTES  =="
echo "======================================="
echo ""

./build/example2/Example2 images/portrait.ppm

echo ""
echo "======================================="
echo "== RUNNING EXAMPLE 3 --  MTCNN DETECTOR + ATTRIBUTES  =="
echo "======================================="
echo ""

./build/example3/Example3 images/portrait.ppm

echo ""
echo ""
