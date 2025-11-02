#!/bin/bash
# build.sh

echo "🔨 Compiling C programs for Linux..."

cd compiler

# Compile the compiler
gcc -o compiler compiler.c -lm
if [ $? -eq 0 ]; then
    echo "✅ Compiler built successfully"
    chmod +x compiler
else
    echo "❌ Failed to build compiler"
    exit 1
fi

# Compile the optimizer
gcc -o optimizer optimizer.c -lm
if [ $? -eq 0 ]; then
    echo "✅ Optimizer built successfully"
    chmod +x optimizer
else
    echo "❌ Failed to build optimizer"
    exit 1
fi

cd ..
echo "✅ All executables built successfully"