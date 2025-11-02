from flask import Flask, request, jsonify
from flask_cors import CORS
from llm.LLM import LLM

app = Flask(__name__)
CORS(app)

# Path to your compiler folder where source.c should go
SOURCE_FILE = "backend/compiler/source.c"

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

        # Write to source.c
        with open(SOURCE_FILE, "w", encoding="utf-8") as f:
            f.write(source_code)
        print(f"📝 Wrote source code to {SOURCE_FILE}")

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
        return jsonify({"message": f"Error: {e}", "success": False}), 500


if __name__ == "__main__":
    print("🧠 LLM Flask API running at http://127.0.0.1:5001/run-llm")
    app.run(host="127.0.0.1", port=5001, debug=True)
