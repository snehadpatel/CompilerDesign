# Python to C Transpiler (Rule-Based)

A lightweight, high-performance Python-to-C transpiler built for simplicity and efficiency. This tool uses a rule-based approach to convert Python scripts into clean, well-indented, and compilable C code.

## 🚀 Features
- **Intelligent Type Inference**: Automatically detects `int` and `float` types based on literals and expressions.
- **Strict Indentation Tracking**: Ensures the generated C code maintains a consistent 4-space indentation for all nested blocks.
- **Smart Entry Point Mapping**: Correctly maps Python's `def main():` to C's `int main() { ... }` and handles `if __name__ == "__main__":` boilerplate.
- **Robust Comment Translation**: Seamlessly converts Python `#` comments to C `//` comments.
- **Input/Output Handling**: Mapped `input()` to `scanf()` and `print()` to `printf()` with correct format specifiers (e.g., `%d`, `%f`, `%s`).
- **Interactive UI**: Built with Streamlit for a seamless user experience, including live code conversion and file downloads.

---

## 🛠️ Requirements
- **Python 3.8+**
- **Streamlit**

---

## 💻 How to Run

### **On macOS (Terminal)**
1. **Open Terminal** and navigate to the project directory:
   ```bash
   cd "path/to/CD_Project"
   ```
2. **Install Dependencies** (if not already installed):
   ```bash
   pip install streamlit
   ```
3. **Run the Application**:
   ```bash
   streamlit run app.py
   ```
4. The application will automatically open in your default browser at `http://localhost:8501`.

### **On Windows (Command Prompt / PowerShell)**
1. **Open CMD or PowerShell** and navigate to the project directory:
   ```cmd
   cd "C:\path\to\CD_Project"
   ```
2. **Install Dependencies**:
   ```cmd
   pip install streamlit
   ```
3. **Run the Application**:
   ```cmd
   python -m streamlit run app.py
   ```
4. If it doesn't open automatically, navigate to `http://localhost:8501` in your browser.

---

## 📂 Project Structure
- `app.py`: Streamlit frontend and application logic.
- `transformer.py`: Core rule-based transformation engine.
- `generator.py`: Final C code template generation.
- `main.py`: Pipeline orchestrator linking the transformer and generator.
- `outputs/`: Directory where generated C files are stored.

---

## 📝 Example Usage
1. Upload a `.py` file or paste your Python code directly into the text area.
2. Click **Convert**.
3. View the transpiled C code in the **C** tab.
4. Download the final `.c` file using the download button.

---

## ⚠️ Limitations
- Only basic Python structures (loops, conditionals, functions, simple assignments) are supported.
- Python-specific complex objects like `list`, `dict`, and `class` are not fully implemented.
- List comprehensions and `try/except` blocks are handled with placeholders to maintain valid C syntax.

---

**Developed for Compiler Design Project (CSE-606)**  
🚀 *Built with speed, simplicity, and robustness in mind.*
