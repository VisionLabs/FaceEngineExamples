#!/bin/bash

echo ""

echo ""
echo "======================================="
echo "== RUNNING EXAMPLE 1 -- VERIFICATION =="
echo "======================================="
echo ""

./build/example1/Example1 LUNASDKExamples/images/Cameron_Diaz.ppm \
LUNASDKExamples/images/Cameron_Diaz_2.ppm 0.7

echo ""
echo "======================================="
echo "== RUNNING EXAMPLE 2 --  ATTRIBUTES  =="
echo "======================================="
echo ""

./build/example2/Example2 LUNASDKExamples/images/portrait.ppm

echo ""
echo "======================================="
echo "== RUNNING EXAMPLE 3 --  MTCNN DETECTOR + ATTRIBUTES  =="
echo "======================================="
echo ""

./build/example3/Example3 LUNASDKExamples/images/portrait.ppm

echo ""
echo "======================================="
echo "== RUNNING EXAMPLE 6 --  LSH  =="
echo "======================================="
echo ""

./build/example6/Example6 LUNASDKExamples/images/Cameron_Diaz.ppm \
LUNASDKExamples/images/ LUNASDKExamples/images_lists/list.txt 0.7

echo ""
echo "======================================="
echo "== RUNNING EXAMPLE 7 --  IO_UTIL  =="
echo "======================================="
echo ""

./build/example7/Example7 LUNASDKExamples/images/portrait.ppm

echo ""
echo "======================================="
echo "== RUNNING EXAMPLE 8 --  IO_UTIL  =="
echo "======================================="
echo ""

./build/example8/Example8 LUNASDKExamples/descriptors/Cameron_Diaz.xpk \
LUNASDKExamples/descriptors/Cameron_Diaz_2.xpk

echo ""
echo ""
