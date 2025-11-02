import os
import subprocess
import requests
import json
import time
import re
from dotenv import load_dotenv

# ============================================================
# CONFIGURATION
# ============================================================
def LLM():
    load_dotenv()

    GEMINI_API_KEY = os.getenv("GEMINI_API_KEY")
    GEMINI_ENDPOINT = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent"

    # Use project directory instead of /tmp
    # Get backend directory (parent of llm directory)
    BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    COMPILER_DIR = os.path.join(BASE_DIR, "compiler")
    
    print(f"📂 Base directory: {BASE_DIR}")
    print(f"📂 Compiler directory: {COMPILER_DIR}")
    
    # Ensure compiler directory exists
    os.makedirs(COMPILER_DIR, exist_ok=True)

    COMPILER_EXECUTABLE_NAME = "compiler"
    OPTIMIZER_EXECUTABLE_NAME = "optimizer"

    COMPILER_EXECUTABLE = os.path.join(COMPILER_DIR, COMPILER_EXECUTABLE_NAME)
    OPTIMIZER_EXECUTABLE = os.path.join(COMPILER_DIR, OPTIMIZER_EXECUTABLE_NAME)

    IR_FILE = os.path.join(COMPILER_DIR, "IR.txt")
    OUTPUT_FILE = os.path.join(COMPILER_DIR, "Output.txt")
    SOURCE_FILE = os.path.join(COMPILER_DIR, "source.c")

    # Create llm directory for reports if it doesn't exist
    LLM_REPORTS_DIR = os.path.join(BASE_DIR, "llm")
    os.makedirs(LLM_REPORTS_DIR, exist_ok=True)
    
    REPORT_TEXT = os.path.join(LLM_REPORTS_DIR, "Gemini_Report.txt")
    REPORT_JSON = os.path.join(LLM_REPORTS_DIR, "Gemini_Review.json")

    # ============================================================
    # CHECK IF EXECUTABLES EXIST
    # ============================================================
    print(f"🔍 Checking for compiler at: {COMPILER_EXECUTABLE}")
    print(f"   Exists: {os.path.exists(COMPILER_EXECUTABLE)}")
    
    print(f"🔍 Checking for optimizer at: {OPTIMIZER_EXECUTABLE}")
    print(f"   Exists: {os.path.exists(OPTIMIZER_EXECUTABLE)}")
    
    # Make executables executable (in case permissions were lost)
    for exe_path in [COMPILER_EXECUTABLE, OPTIMIZER_EXECUTABLE]:
        if os.path.exists(exe_path):
            try:
                os.chmod(exe_path, 0o755)
                print(f"✅ Set executable permissions for {exe_path}")
            except Exception as e:
                print(f"⚠️ Could not set permissions for {exe_path}: {e}")

    # ============================================================
    # RUN COMPILER EXECUTABLE TO GENERATE OUTPUT FILE
    # ============================================================

    """
    def run_c_compiler():
        print("⚙️ Running C compiler to generate IR.txt...\n")

        if not os.path.exists(COMPILER_EXECUTABLE):
            print(f"❌ Compiler executable not found: {COMPILER_EXECUTABLE}")
            return False

        if not os.path.exists(SOURCE_FILE):
            print(f"❌ Source file not found: {SOURCE_FILE}")
            return False

        try:
            # Run compiler
            compiled = subprocess.run(
                [COMPILER_EXECUTABLE],
                cwd=COMPILER_DIR,
                capture_output=True,
                text=True,
                timeout=45
            )

            print("🔧 Compiler stdout:\n", compiled.stdout)
            print("⚠️ Compiler stderr:\n", compiled.stderr)
            print(f"🔧 Compiler return code: {compiled.returncode}")

            # Wait for IR.txt to appear
            for i in range(10):
                if os.path.exists(IR_FILE):
                    print("✅ IR.txt found!")
                    return True
                print(f"⏳ Waiting for IR.txt... ({i+1}/10)")
                time.sleep(1)

            print("❌ IR.txt not found after running compiler.")
            return False

        except subprocess.TimeoutExpired:
            print("❌ Compiler process timed out.")
            return False
        except Exception as e:
            print(f"❌ Error running compiler: {e}")
            import traceback
            traceback.print_exc()
            return False
    """
    def run_c_compiler():
        """Run the compiled C compiler and wait for IR.txt to appear."""
        print("⚙️ Running C compiler to generate IR.txt...\n")

        # Check if executable exists and is actually executable
        if not os.path.exists(COMPILER_EXECUTABLE):
            print(f"❌ Compiler executable not found: {COMPILER_EXECUTABLE}")
            return False
        
        # Check if it's actually a file and executable
        import stat
        file_stat = os.stat(COMPILER_EXECUTABLE)
        is_executable = bool(file_stat.st_mode & stat.S_IXUSR)
        print(f"📋 Compiler file info:")
        print(f"   Path: {COMPILER_EXECUTABLE}")
        print(f"   Exists: {os.path.exists(COMPILER_EXECUTABLE)}")
        print(f"   Is file: {os.path.isfile(COMPILER_EXECUTABLE)}")
        print(f"   Executable bit: {is_executable}")
        print(f"   Size: {file_stat.st_size} bytes")
        print(f"   Permissions: {oct(file_stat.st_mode)}")

        # Check source file
        if not os.path.exists(SOURCE_FILE):
            print(f"❌ Source file not found: {SOURCE_FILE}")
            return False

        # Read and display source file
        try:
            with open(SOURCE_FILE, "r") as f:
                source_content = f.read()
            print(f"\n📄 Source file content ({len(source_content)} bytes):")
            print("=" * 60)
            print(source_content[:500])  # First 500 chars
            if len(source_content) > 500:
                print("... (truncated)")
            print("=" * 60)
        except Exception as e:
            print(f"⚠️ Could not read source file: {e}")
            return False

        # List all files in directory before running
        print(f"\n📂 Files in {COMPILER_DIR} BEFORE compilation:")
        try:
            for item in sorted(os.listdir(COMPILER_DIR)):
                item_path = os.path.join(COMPILER_DIR, item)
                if os.path.isfile(item_path):
                    size = os.path.getsize(item_path)
                    print(f"   {item:30s} ({size:>10,} bytes)")
        except Exception as e:
            print(f"   Error listing directory: {e}")

        # Check if compiler is a valid ELF file (Linux executable)
        try:
            with open(COMPILER_EXECUTABLE, "rb") as f:
                magic = f.read(4)
                if magic[:4] == b'\x7fELF':
                    print(f"✅ Compiler is a valid Linux ELF executable")
                else:
                    print(f"❌ Compiler is NOT a valid ELF executable!")
                    print(f"   Magic bytes: {magic.hex()}")
                    print(f"   This might be a script or wrong architecture")
                    return False
        except Exception as e:
            print(f"⚠️ Could not check executable format: {e}")

        # Try using 'file' command to identify the executable
        try:
            file_check = subprocess.run(
                ["file", COMPILER_EXECUTABLE],
                capture_output=True,
                text=True,
                timeout=5
            )
            print(f"🔍 File type check: {file_check.stdout.strip()}")
        except Exception as e:
            print(f"⚠️ Could not run 'file' command: {e}")

        # Now try to run the compiler
        print(f"\n🔧 Attempting to run: {COMPILER_EXECUTABLE}")
        print(f"   Working directory: {COMPILER_DIR}")
        
        try:
            # First attempt - just the executable
            compiled = subprocess.run(
                [COMPILER_EXECUTABLE],
                cwd=COMPILER_DIR,
                capture_output=True,
                text=True,
                timeout=45
            )

            print(f"\n📊 Compiler execution results:")
            print(f"   Return code: {compiled.returncode}")
            print(f"   Stdout length: {len(compiled.stdout)} chars")
            print(f"   Stderr length: {len(compiled.stderr)} chars")
            
            if compiled.stdout:
                print("\n🔧 Compiler stdout:")
                print("-" * 60)
                print(compiled.stdout)
                print("-" * 60)
            else:
                print("\n🔧 Compiler stdout: (empty)")
                
            if compiled.stderr:
                print("\n⚠️ Compiler stderr:")
                print("-" * 60)
                print(compiled.stderr)
                print("-" * 60)
            else:
                print("\n⚠️ Compiler stderr: (empty)")

            # List files after compilation attempt
            print(f"\n📂 Files in {COMPILER_DIR} AFTER compilation attempt:")
            try:
                for item in sorted(os.listdir(COMPILER_DIR)):
                    item_path = os.path.join(COMPILER_DIR, item)
                    if os.path.isfile(item_path):
                        size = os.path.getsize(item_path)
                        print(f"   {item:30s} ({size:>10,} bytes)")
            except Exception as e:
                print(f"   Error listing directory: {e}")

            # Check if compilation failed
            if compiled.returncode != 0:
                print(f"\n❌ Compiler exited with error code {compiled.returncode}")
                
                # Try to understand why it failed
                if not compiled.stdout and not compiled.stderr:
                    print("⚠️ No output from compiler - might be a linking or runtime error")
                    
                    # Try running with strace to see what's happening
                    print("\n🔍 Attempting to trace execution...")
                    try:
                        trace = subprocess.run(
                            ["strace", "-e", "trace=open,openat,read", COMPILER_EXECUTABLE],
                            cwd=COMPILER_DIR,
                            capture_output=True,
                            text=True,
                            timeout=10
                        )
                        print("📝 System call trace:")
                        print(trace.stderr[:1000])  # Show first 1000 chars of trace
                    except Exception as e:
                        print(f"   Could not trace: {e}")
                
                return False

            # Wait for IR.txt to appear
            print(f"\n⏳ Waiting for IR.txt to be created...")
            for i in range(10):
                if os.path.exists(IR_FILE):
                    print(f"✅ IR.txt found after {i+1} second(s)!")
                    
                    # Display IR.txt content
                    try:
                        with open(IR_FILE, "r") as f:
                            ir_content = f.read()
                        print(f"\n📄 IR.txt content ({len(ir_content)} bytes):")
                        print("=" * 60)
                        print(ir_content[:500])  # First 500 chars
                        if len(ir_content) > 500:
                            print("... (truncated)")
                        print("=" * 60)
                    except Exception as e:
                        print(f"⚠️ Could not read IR.txt: {e}")
                    
                    return True
                
                print(f"   Attempt {i+1}/10 - IR.txt not found yet")
                time.sleep(1)

            print("\n❌ IR.txt was never created")
            print("   The compiler ran but didn't produce output")
            return False

        except subprocess.TimeoutExpired:
            print("❌ Compiler process timed out (took more than 45 seconds)")
            return False
        except FileNotFoundError as e:
            print(f"❌ Could not execute compiler: {e}")
            print("   The file exists but cannot be executed")
            return False
        except Exception as e:
            print(f"❌ Unexpected error running compiler: {e}")
            import traceback
            traceback.print_exc()
            return False
    def run_c_optimizer():
        """Run the optimizer and wait for Output.txt to appear."""
        print("⚙️ Running C optimizer to generate Output.txt...\n")

        if not os.path.exists(OPTIMIZER_EXECUTABLE):
            print(f"❌ Optimizer executable not found: {OPTIMIZER_EXECUTABLE}")
            return False

        if not os.path.exists(IR_FILE):
            print(f"❌ IR.txt not found: {IR_FILE}")
            return False

        try:
            result = subprocess.run(
                [OPTIMIZER_EXECUTABLE],
                cwd=COMPILER_DIR,
                capture_output=True,
                text=True,
                timeout=45
            )

            print("🔧 Optimizer stdout:\n", result.stdout)
            print("⚠️ Optimizer stderr:\n", result.stderr)
            print(f"🔧 Optimizer return code: {result.returncode}")

            # Wait for Output.txt to appear
            for i in range(10):
                if os.path.exists(OUTPUT_FILE):
                    print("✅ Output.txt found!")
                    return True
                print(f"⏳ Waiting for Output.txt... ({i+1}/10)")
                time.sleep(1)

            print("❌ Output.txt not found after running optimizer.")
            return False

        except subprocess.TimeoutExpired:
            print("❌ Optimizer process timed out.")
            return False
        except Exception as e:
            print(f"❌ Error running optimizer: {e}")
            import traceback
            traceback.print_exc()
            return False

    # ============================================================
    # GEMINI API CALL
    # ============================================================

    def call_gemini_api(prompt, retries=5, delay=5):
        headers = {
            "Content-Type": "application/json",
            "x-goog-api-key": GEMINI_API_KEY
        }

        body = {
            "contents": [{"parts": [{"text": prompt}]}],
            "generationConfig": {
                "maxOutputTokens": 6000,
                "temperature": 0.3
            }
        }

        for attempt in range(1, retries + 1):
            response = requests.post(GEMINI_ENDPOINT, headers=headers, json=body)

            if response.status_code == 200:
                data = response.json()
                try:
                    candidates = data.get("candidates", [])
                    if not candidates:
                        return "⚠️ No response candidates found.", data

                    content = candidates[0].get("content", {})
                    parts = content.get("parts", [])
                    all_text = [p["text"] for p in parts if "text" in p]

                    if not all_text and "output_text" in candidates[0]:
                        all_text.append(candidates[0]["output_text"])

                    final_text = "\n".join(all_text) if all_text else "⚠️ Gemini returned no readable text."
                    return final_text, data

                except Exception as e:
                    return f"⚠️ Parsing error: {e}\nRaw data:\n{json.dumps(data, indent=2)}", data

            elif response.status_code in (429, 503):
                print(f"⚠️ Gemini busy (status {response.status_code}). Retrying in {delay}s... ({attempt}/{retries})")
                time.sleep(delay)
                delay *= 2
                continue
            else:
                raise Exception(f"Gemini API Error {response.status_code}: {response.text}")

        raise Exception("Gemini API unavailable after several retries.")

    # ============================================================
    # PARSING HELPERS
    # ============================================================

    def extract_summary(review_text):
        lines = review_text.strip().splitlines()
        summary_line = ""
        for line in reversed(lines):
            if "✅" in line or "⚠️" in line:
                summary_line = line.strip()
                break

        if "✅" in summary_line:
            status = "Optimization Correct"
        elif "⚠️" in summary_line:
            status = "Issues Found"
        else:
            status = "Unclear"

        return {"summary": summary_line, "status": status}

    def extract_suggestions(review_text):
        return re.findall(r"[-*•]\s+(.*)", review_text)

    def extract_tac_code(review_text):
        """
        Extracts the optimized TAC code section (if Gemini includes it).
        Looks for headings like 'Optimization' or 'Optimized Code' and captures the code block.
        """
        # Try common section headers Gemini might use
        match = re.search(
            r"\$?Optimization:[:\n\s]*([\s\S]+?)(?:\n[-*•]|✅|⚠️|$)",
            review_text,
            re.IGNORECASE
        )

        if match:
            tac_code = match.group(1).strip()
            # Remove any extra markdown or formatting artifacts
            tac_code = re.sub(r"```[a-zA-Z]*|```", "", tac_code).strip()
            return tac_code
        return None


    # ============================================================
    # GEMINI REVIEW FUNCTION
    # ============================================================

    def review_output_file():
        if not os.path.exists(OUTPUT_FILE):
            print("❌ Output.txt not found! Run your optimizer first.")
            return {"success": False, "message": "Output.txt not found"}

        with open(OUTPUT_FILE, "r") as f:
            optimized_code = f.read()

        prompt = f"""
You are an expert compiler engineer reviewing optimized three-address code (TAC).

Analyze the code below and respond briefly:
1. Verify if semantics are preserved (no logic change).
2. Mention unsafe optimizations, if any.
3. Suggest at most 3 further optimizations (clear, one-liners).
4. Use short bullet points only — no long explanations.
5. End with one summary line:
→ "✅ Optimization Correct" or "⚠️ Issues Found: <reason>"

--- Optimized TAC ---
{optimized_code}

Format your output as:
- ✅/⚠️ statements
- 2–4 bullet points only
Keep the total output under 6 lines (suitable for frontend card view).
Dont use any emojis.
Also compulsorily generate your version of the optimized three address code with the heading: "$Optimization:$" in the end of the summary.
All the optimized code must be in newlines. 
"""

        print("🤖 Sending optimized TAC to Gemini...\n")
        review_text, raw_json = call_gemini_api(prompt)

        print("\n=== Gemini Review ===\n")
        print(review_text)

        summary_info = extract_summary(review_text)
        suggestions = extract_suggestions(review_text)
        optimized_tac = extract_tac_code(review_text)
        
        # Save plain text
        with open(REPORT_TEXT, "w", encoding="utf-8") as f:
            f.write(review_text)

        # Save structured JSON
        structured_output = {
            "success": True,
            "summary": summary_info["summary"],
            "status": summary_info["status"],
            "suggestions": suggestions,
            "full_text": review_text,
            "optimized_code": optimized_tac,
            "unoptimized_code": optimized_code
        }

        with open(REPORT_JSON, "w", encoding="utf-8") as f:
            json.dump(structured_output, f, indent=2)

        print(f"📦 JSON review saved to {REPORT_JSON}")
        return structured_output

    # ============================================================
    # MAIN EXECUTION LOGIC
    # ============================================================
    print("========================================================")
    print("  Gemini Review Automation for C Optimizer Project       ")
    print("========================================================\n")

    if run_c_compiler() and run_c_optimizer():
        json_result = review_output_file()
        print("\n✅ Final structured JSON ready for frontend:\n")
        print(json.dumps(json_result, indent=2))
        return json_result
    else:
        return {"success": False, "message": "Compiler or optimizer failed to run."}
