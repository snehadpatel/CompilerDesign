import subprocess
import os

def run_compiler(code, filename):
    """
    Bridge function to invoke the C-based py2c transpiler.
    1. Writes code to a temporary file.
    2. Runs ./py2c.
    3. Reads and returns the generated C code.
    """
    # Ensure py2c is built
    if not os.path.exists("./py2c"):
        return "Error: py2c binary not found. Please run 'make' first.", None

    # Write input to a temp file
    temp_py = f"ui/{filename}.py"
    with open(temp_py, "w") as f:
        f.write(code)

    try:
        # Run the transpiler
        # py2c writes its output to outputs/output.c
        result = subprocess.run(
            ["./py2c", temp_py],
            capture_output=True,
            text=True,
            check=False
        )

        if result.returncode != 0:
            # Transpiler error
            return f"Transpilation Error:\n{result.stdout}\n{result.stderr}", None

        # Success - read the output
        output_path = "outputs/output.c"
        if os.path.exists(output_path):
            with open(output_path, "r") as f:
                c_code = f.read()
            return c_code, output_path
        else:
            return "Error: Transpiler finished but outputs/output.c was not found.", None

    except Exception as e:
        return f"System Error: {str(e)}", None
    finally:
        # Clean up temp file
        if os.path.exists(temp_py):
            os.remove(temp_py)
