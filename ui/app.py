import streamlit as st
import os
from bridge import run_compiler

# Hide menu
st.markdown("""
<style>
#MainMenu {visibility: hidden;}
header {visibility: hidden;}
footer {visibility: hidden;}
</style>
""", unsafe_allow_html=True)

st.title("Python to C Converter")

# Initialize session state
if "code" not in st.session_state:
    st.session_state.code = ""

if "converted" not in st.session_state:
    st.session_state.converted = False

if "filename" not in st.session_state:
    st.session_state.filename = "manual_input"

#  Upload file
uploaded_file = st.file_uploader("Upload .py file", type=["py"])

if uploaded_file is not None:
    # ✅ Update ONLY when new file uploaded
    file_content = uploaded_file.read().decode("utf-8")

    st.session_state.code = file_content
    st.session_state.filename = os.path.splitext(uploaded_file.name)[0]

# Text editor (NOW SYNCS CORRECTLY)
st.session_state.code = st.text_area(
    "Enter Python Code",
    value=st.session_state.code,
    height=300
)

# Convert button
if st.button("Convert"):

    if not st.session_state.code.strip():
        st.error("No code provided")

    else:
        result, path = run_compiler(
            st.session_state.code,
            st.session_state.filename
        )

        if path is None:
            st.error(result)
            st.session_state.converted = False
        else:
            st.session_state.c_code = result
            st.session_state.converted = True

# Show output
if st.session_state.converted:

    tab1, tab2 = st.tabs(["Python", "C"])

    with tab1:
        st.code(st.session_state.code, language="python")

        st.download_button(
            "Download Python File",
            data=st.session_state.code,
            file_name=f"{st.session_state.filename}.py",
            mime="text/x-python",
            key="python_download"
        )

    with tab2:
        st.code(st.session_state.c_code, language="c")

        st.download_button(
            "Download C File",
            data=st.session_state.c_code,
            file_name=f"{st.session_state.filename}.c",
            mime="text/x-csrc",
            key="c_download"
        )