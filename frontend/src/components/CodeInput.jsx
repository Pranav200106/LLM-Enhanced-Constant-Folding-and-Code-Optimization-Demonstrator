import { useState, useRef } from "react";

function CodeInput({ onOptimize }) {
  const [code, setCode] = useState("");

  const [fileName, setFileName] = useState("");
  const textareaRef = useRef(null);

  const handleFileUpload = async (e) => {
    const file = e.target.files[0];
    if (!file) return;

    setFileName(file.name);

    const reader = new FileReader();
    reader.onload = (event) => {
      const content = event.target.result;
      setCode(content);
      console.log(content)
      adjustTextareaHeight(content);
    };
    reader.readAsText(file);
  };

  const adjustTextareaHeight = (value) => {
    const textarea = textareaRef.current;
    if (textarea) {
      textarea.style.height = "auto";
      textarea.style.height = textarea.scrollHeight + "px";
    }
  };

  const handleInput = (e) => {
    const value = e.target.value;
    setCode(value);
    adjustTextareaHeight(value);
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
          <h2 className="text-lg font-semibold mb-4">Paste Your Code</h2>

          <textarea
            ref={textareaRef}
            rows={1}
            className="textarea w-full h-96 font-mono resize-none overflow-hidden rounded-lg"
            value={code}
            onInput={handleInput}
            placeholder="Paste or write your C/C++ code here..."
          />

          <button
            className="btn btn-primary mt-4"
            onClick={() => onOptimize(code)}
          >
            Optimize Code
          </button>
        </div>

        {/* === File Upload Tab === */}
        <input
          type="radio"
          name="inputs"
          className="tab"
          aria-label="File Upload"
        />
        <div className="tab-content border-base-300 bg-base-100 p-10 flex-col">
          <h2 className="text-lg font-semibold mb-4">
            Upload Your Source File
          </h2>
          <div>
            <input
              type="file"
              accept=".c,.cpp,.txt"
              className="file-input file-input-bordered w-full max-w-xs"
              onChange={handleFileUpload}
            />

            {fileName && (
              <p className="mt-2 text-sm text-gray-500">
                Loaded file: <strong>{fileName}</strong>
              </p>
            )}
          </div>

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
