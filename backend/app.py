from flask import Flask, request, jsonify
from flask_cors import CORS
from llm.LLM import LLM
import os

app = Flask(__name__)
CORS(app)

COMPILER_DIR = os.path.join(os.path.dirname(__file__), "compiler")
SOURCE_FILE = os.path.join(COMPILER_DIR, "source.c")

@app.route("/")
def home():
    return "Flask backend is running successfully!"

@app.route("/run-llm", methods=["POST"])
def run_llm():
    try:
        # Get code from request body (JSON or plain text)
        if request.is_json:
            data = request.get_json()
            source_code = data.get("code", "")
        else:
            source_code = request.data.decode("utf-8")

        if not source_code.strip():
            return jsonify({"success": False, "message": "No source code provided."}), 400

        # Ensure compiler directory exists
        os.makedirs(COMPILER_DIR, exist_ok=True)
        
        # Remove old files to ensure fresh compilation
        old_files = [SOURCE_FILE, 
                     os.path.join(COMPILER_DIR, "IR.txt"),
                     os.path.join(COMPILER_DIR, "Output.txt")]
        
        for old_file in old_files:
            if os.path.exists(old_file):
                os.remove(old_file)
                print(f"🗑️ Removed old file: {old_file}")

        # Write to source.c with explicit flush and sync
        print(f"📝 Writing source code to {SOURCE_FILE}")
        print(f"📄 Source code length: {len(source_code)} characters")
        
        with open(SOURCE_FILE, "w", encoding="utf-8") as f:
            f.write(source_code)
            f.flush()  # Force write to disk
            os.fsync(f.fileno())  # Ensure OS writes to disk
        
        # Verify the file was written correctly
        if os.path.exists(SOURCE_FILE):
            with open(SOURCE_FILE, "r", encoding="utf-8") as f:
                written_content = f.read()
            print(f"✅ File written successfully. Size: {len(written_content)} bytes")
            
            # Check if content matches
            if written_content == source_code:
                print("✅ Content verification successful!")
            else:
                print("⚠️ Content mismatch detected!")
        else:
            print(f"❌ File not found after writing: {SOURCE_FILE}")
            return jsonify({"success": False, "message": "Failed to write source file"}), 500

        # Run the full LLM pipeline
        print("🚀 Running LLM review pipeline...")
        result = LLM()  # Waits until everything finishes

        # Return result to frontend
        if isinstance(result, dict):
            print(result)
            return jsonify(result)
        else:
            return jsonify({
                "message": "No valid JSON result returned from LLM.",
                "success": False
            }), 500

    except Exception as e:
        print(f"❌ Error in /run-llm: {e}")
        import traceback
        traceback.print_exc()
        return jsonify({"message": f"Error: {str(e)}", "success": False}), 500


if __name__ == "__main__":
    print("🧠 LLM Flask API running at http://127.0.0.1:5001/run-llm")
    app.run(host="0.0.0.0", port=10000)
    #app.run(host="127.0.0.1", port=6000)