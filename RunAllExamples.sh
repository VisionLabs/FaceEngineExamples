#!/bin/bash

echo ""

echo ""
echo "======================================="
echo "== RUNNING EXAMPLE 1 -- VERIFICATION =="
echo "======================================="
echo ""

./build/example1/Example1 portrait.ppm portrait.ppm 0.7

echo ""
echo "======================================="
echo "== RUNNING EXAMPLE 2 --  ATTRIBUTES  =="
echo "======================================="
echo ""

./build/example2/Example2 portrait.ppm

echo ""
echo "======================================="
echo "== RUNNING EXAMPLE 3 --  MTCNN  =="
echo "======================================="
echo ""

./build/example3/Example3 portrait.ppm

echo ""
echo ""
