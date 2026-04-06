# 🖥️ Transpiler Web Interface

This directory contains the Streamlit-based web interface for the Python-to-C Transpiler. It provides a visual, interactive way to perform source-to-source translation without using the terminal.

---

## 🎨 Features

- **Interactive Editor**: Write and edit Python code directly in the browser.
- **File Upload**: Upload existing `.py` files for batch conversion.
- **Instant Preview**: View Python and generated C code side-by-side.
- **Downloadable Outputs**: Download your converted `.c` files immediately.

---

## 🏗️ Architecture

The UI acts as a lightweight wrapper (`app.py`) that communicates with the C-based transpiler core via a Python bridge (`bridge.py`).

1. **Input**: User provides Python source via text area or file upload.
2. **Bridge**: `bridge.py` writes temporary files and executes the compiled `./py2c` binary.
3. **Output**: The bridge captures the generated C code and returns it to the Streamlit session.

---

## 🚀 Local Installation

To run the UI on your local machine:

1. **Install Dependencies**:
   ```bash
   pip install -r requirements.txt
   ```

2. **Launch with Helper Script**:
   ```bash
   bash run_ui.sh
   ```

   *Alternatively, via Streamlit directly:*
   ```bash
   streamlit run ui/app.py
   ```

---

## 📦 Requirements

- **Python 3.8+**
- **Streamlit**
- **GCC/Make** (to compile the core transpiler)
