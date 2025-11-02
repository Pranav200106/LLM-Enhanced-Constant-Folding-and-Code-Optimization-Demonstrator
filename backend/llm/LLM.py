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

    COMPILER_DIR = os.path.join("/tmp", "compiler")
    os.makedirs(COMPILER_DIR, exist_ok=True)

    COMPILER_EXECUTABLE_NAME = "compiler"
    OPTIMIZER_EXECUTABLE_NAME = "optimizer"

    COMPILER_EXECUTABLE = os.path.join(COMPILER_DIR, COMPILER_EXECUTABLE_NAME)
    OPTIMIZER_EXECUTABLE = os.path.join(COMPILER_DIR, OPTIMIZER_EXECUTABLE_NAME)

    IR_FILE = os.path.join(COMPILER_DIR, "IR.txt")
    OUTPUT_FILE = os.path.join(COMPILER_DIR, "Output.txt")
    SOURCE_FILE = os.path.join(COMPILER_DIR, "source.c")

    # Create llm directory if it doesn't exist
    LLM_DIR = os.path.join(os.path.dirname(__file__), "..")
    os.makedirs(os.path.join(LLM_DIR, "llm"), exist_ok=True)
    
    REPORT_TEXT = os.path.join(LLM_DIR, "llm", "Gemini_Report.txt")
    REPORT_JSON = os.path.join(LLM_DIR, "llm", "Gemini_Review.json")

    # ============================================================
    # RUN COMPILER EXECUTABLE TO GENERATE OUTPUT FILE
    # ============================================================
    import shutil

    # Copy executables from repo to /tmp/compiler if not already there
    # Get the directory where LLM.py is located, then go to parent (backend), then to compiler
    REPO_COMPILER_DIR = os.path.join(os.path.dirname(os.path.dirname(__file__)), "compiler")
    
    print(f"📂 Looking for executables in: {REPO_COMPILER_DIR}")

    for exe in ["compiler", "optimizer"]:
        src = os.path.join(REPO_COMPILER_DIR, exe)
        dst = os.path.join(COMPILER_DIR, exe)
        
        print(f"🔍 Checking if {src} exists: {os.path.exists(src)}")
        
        if os.path.exists(src):
            shutil.copy(src, dst)
            os.chmod(dst, 0o755)  # make executable
            print(f"✅ Copied {exe} to {dst}")
        else:
            print(f"⚠️ Source executable not found: {src}")

    def run_c_compiler():
        """Run the compiled C optimizer and wait for Output.txt to appear."""
        print("⚙️ Running C optimizer to generate Output.txt...\n")

        try:
            # Run compiler (update args if needed)
            compiled = subprocess.run(
                [COMPILER_EXECUTABLE],
                cwd=COMPILER_DIR,
                capture_output=True,
                text=True,
                timeout=45
            )

            print("🔧 Compiler stdout:\n", compiled.stdout)
            print("⚠️ Compiler stderr:\n", compiled.stderr)

            for _ in range(10):
                if os.path.exists(IR_FILE):
                    print("✅ IR.txt found!")
                    return True
                time.sleep(1)

            print("❌ IR.txt not found after running compiler.")
            return False

        except subprocess.TimeoutExpired:
            print("❌ Compiler process timed out.")
            return False
        except FileNotFoundError:
            print(f"❌ Compiler executable not found: {os.path.join(COMPILER_DIR, COMPILER_EXECUTABLE_NAME)}")
            return False

    def run_c_optimizer():
        try:
            result = subprocess.run(
                [OPTIMIZER_EXECUTABLE],
                cwd=COMPILER_DIR,
                capture_output=True,
                text=True,
                timeout=45
            )

            print("🔧 Compiler stdout:\n", result.stdout)
            print("⚠️ Compiler stderr:\n", result.stderr)

            for _ in range(10):
                if os.path.exists(OUTPUT_FILE):
                    print("✅ Output.txt found!")
                    return True
                time.sleep(1)

            print("❌ Output.txt not found after running compiler.")
            return False

        except subprocess.TimeoutExpired:
            print("❌ Optimizer process timed out.")
            return False
        except FileNotFoundError:
            print(f"❌ Optimizer executable not found: {os.path.join(COMPILER_DIR, OPTIMIZER_EXECUTABLE_NAME)}")
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
            return

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
