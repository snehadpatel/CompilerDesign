import subprocess
import os

def run_compiler(code, filename):
    """
    Bridge function to invoke the C-based py2c transpiler.
    1. Writes code to a temporary file.
    2. Runs ./py2c (builds if needed).
    3. Reads and returns the generated C code.
    """
    # Get the project root directory (parent of ui/)
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    os.chdir(project_root)

    # Ensure py2c is built
    if not os.path.exists("./py2c"):
        # Try to build it
        build_result = subprocess.run(
            ["make"],
            capture_output=True,
            text=True,
            check=False
        )
        if build_result.returncode != 0:
            return f"Error: Failed to build py2c binary.\n{build_result.stdout}\n{build_result.stderr}", None

    # Ensure outputs directory exists
    os.makedirs("outputs", exist_ok=True)

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
