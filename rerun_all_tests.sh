#!/bin/bash

# Build the transpiler
make clean && make
if [ $? -ne 0 ]; then
    echo "❌ Failed to build py2c"
    exit 1
fi

mkdir -p outputs

# List of tests to run
TESTS=$(ls tests/*.py | grep -v "test_error.py")

for test_file in $TESTS; do
    base=$(basename "$test_file" .py)
    echo "------------------------------------------------"
    echo "🔍 Testing $test_file..."
    
    # Transpile
    ./py2c "$test_file" > /dev/null
    if [ $? -ne 0 ]; then
        echo "❌ Transpilation failed for $test_file"
        continue
    fi
    
    # Save the output
    out_file="outputs/${base#test_}_output.c"
    mv outputs/output.c "$out_file"
    
    # Compile the C code
    # Use -lm for math library
    gcc "$out_file" -o test_bin -lm 2> gcc_errors.log
    if [ $? -ne 0 ]; then
        echo "❌ GCC Compilation failed for $out_file"
        cat gcc_errors.log
        continue
    fi
    
    # Run the binary
    # For test_simple.py, we need an input
    if [[ "$test_file" == *"test_simple.py"* ]]; then
        output=$(echo "15" | ./test_bin)
    else
        output=$(./test_bin)
    fi
    
    echo "✅ Success! Output:"
    echo "$output"
done

echo "------------------------------------------------"
echo "Done with all tests."
rm -f test_bin gcc_errors.log
