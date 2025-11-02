import { useState, useRef } from "react";

function CodeInput({ onOptimize }) {
  const [code, setCode] = useState("");

  const [fileName, setFileName] = useState("");
  const textareaRef = useRef(null);

  // Handle file upload
  const handleFileUpload = async (e) => {
    const file = e.target.files[0];
    if (!file) return;
    setFileName(file.name);

    const reader = new FileReader();
    reader.onload = (event) => {
      setCode(event.target.result);
    };
    reader.readAsText(file);
  };

  // Dynamically adjust textarea height
  const handleInput = (e) => {
    const textarea = textareaRef.current;
    textarea.style.height = "auto";
    textarea.style.height = textarea.scrollHeight + "px";
    setCode(e.target.value);
  };

  // Load a sample program
  const handleLoadSample = () => {
    const sample = `int main() {
    int x = 2;
    int y = 3;
    int z = (x + y) * 5 - (y / 2);
    return z;
}`;
    setCode(sample);

    // auto-resize textarea after setting
    const textarea = textareaRef.current;
    if (textarea) {
      setTimeout(() => {
        textarea.style.height = "auto";
        textarea.style.height = textarea.scrollHeight + "px";
      }, 0);
    }
  };

  return (
    <div className="card bg-base-100 shadow-xl p-6 w-full">
      <div className="tabs tabs-border">
        {/* === Paste Code Tab === */}
        <input
          type="radio"
          name="inputs"
          className="tab"
          aria-label="Paste Code"
          defaultChecked
        />
        <div className="tab-content border-base-300 bg-base-100 p-10">
          <h2 className="text-lg font-semibold mb-2">Input Code</h2>
          <textarea
            ref={textareaRef}
            rows={8}
            className="textarea textarea-bordered w-full font-mono rounded-lg"
            value={code}
            placeholder="Paste your C code here..."
            onChange={handleInput}
          />

          <div className="flex flex-wrap gap-3 mt-4">
            <button className="btn btn-primary" onClick={() => onOptimize(code)}>
              Optimize Code
            </button>

            <button
              className="btn btn-outline btn-secondary"
              onClick={handleLoadSample}
            >
              Load Sample Code
            </button>
          </div>
        </div>

        {/* === File Upload Tab === */}
        <input
          type="radio"
          name="inputs"
          className="tab"
          aria-label="File Upload"
        />
        <div className="tab-content border-base-300 bg-base-100 p-10 flex-col">
          <h2 className="text-lg font-semibold mb-2">Upload your source code</h2>
          <div>
            <input
              type="file"
              accept=".c,.cpp,.txt"
              className="file-input w-full max-w-xs"
              onChange={handleFileUpload}
            />
          </div>

          {fileName && (
            <p className="mt-2 text-sm text-gray-500">
              Loaded file: <strong>{fileName}</strong>
            </p>
          )}

          <button
            className="btn btn-primary mt-4"
            onClick={() => onOptimize(code)}
          >
            Optimize Code
          </button>
        </div>
      </div>
    </div>
  );
}

export default CodeInput;
