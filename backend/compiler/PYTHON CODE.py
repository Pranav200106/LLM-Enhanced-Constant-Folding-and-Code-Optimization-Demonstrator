import os
import requests
import json
import time
import re

# ============================================================
# CONFIGURATION
# ============================================================

GEMINI_API_KEY = os.getenv("GEMINI_API_KEY", "AIzaSyCQDSyTtxoThaEWmJcxOPG5BFCIUDCezDI")
GEMINI_ENDPOINT = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent"
OUTPUT_FILE = "Output.txt"
REPORT_TEXT = "Gemini_Report.txt"
REPORT_JSON = "Gemini_Review.json"

# ============================================================
# GEMINI API CALL — RETURNS TEXT + RAW JSON
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
                all_text = []

                for p in parts:
                    if isinstance(p, dict) and "text" in p:
                        all_text.append(p["text"])

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
# GEMINI REVIEW FUNCTION
# ============================================================

def extract_summary(review_text):
    """Extract final summary line and status for web display."""
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
    """Extract suggestion bullet points for web UI."""
    suggestions = re.findall(r"[-*•]\s+(.*)", review_text)
    return suggestions


def review_output_file():
    if not os.path.exists(OUTPUT_FILE):
        print("❌ Output.txt not found! Run your optimizer first.")
        return

    with open(OUTPUT_FILE, "r") as f:
        optimized_code = f.read()

    prompt = f"""
You are an expert compiler engineer and code optimization reviewer.
Analyze the following **optimized three-address code (TAC)**.
Your suggestions will be displayed in a web application, so make them clear, short, and structured.

--- OPTIMIZED CODE ---
{optimized_code}

Please:
1. Verify if the optimization preserves semantics (i.e., no logic changes).
2. Point out if any optimization step (constant folding, dead code elimination, etc.) seems unsafe.
3. Suggest further compiler-level optimizations.
4. Write your explanation in a clear, human style with bullet points.
5. End with one summary line:
   → "✅ Optimization Correct" or "⚠️ Issues Found" with a short reason.
"""

    print("🤖 Sending optimized TAC to Gemini for review...\n")
    review_text, raw_json = call_gemini_api(prompt)

    print("\n=== Gemini Review (Text) ===\n")
    print(review_text)

    # Extract summary + suggestions
    summary_info = extract_summary(review_text)
    suggestions = extract_suggestions(review_text)

    # Save plain text
    with open(REPORT_TEXT, "w", encoding="utf-8") as f:
        f.write("=== Gemini Review ===\n\n")
        f.write(review_text)

    print(f"\n📝 Saved human-readable review to '{REPORT_TEXT}'")

    # Save structured JSON (for web)
    structured_output = {
        "summary": summary_info["summary"],
        "status": summary_info["status"],
        "suggestions": suggestions,
        "full_text": review_text
    }

    with open(REPORT_JSON, "w", encoding="utf-8") as f:
        json.dump(structured_output, f, indent=2)

    print(f"📦 Saved web-ready JSON review to '{REPORT_JSON}'")


# ============================================================
# MAIN
# ============================================================

if __name__ == "__main__":
    print("========================================================")
    print("  Gemini Review for Optimized Three-Address Code (Web)  ")
    print("========================================================\n")
    review_output_file()
