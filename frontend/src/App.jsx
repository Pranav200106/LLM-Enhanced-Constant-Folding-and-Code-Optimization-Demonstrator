import { useState } from "react";
import Navbar from "./components/NavBar";
import CodeInput from "./components/CodeInput";
import OutputPanel from "./components/OutputPanel";

function App() {
  const [optimizedCode, setOptimizedCode] = useState("");
  const [optimizationLog, setOptimizationLog] = useState([]);
  const [status, setStatus] = useState("");
  const [unOptimizedCode, setUnOptimizedCode] = useState("");
  const [fullText, setFullText] = useState("");
  const [loading, setLoading] = useState(false);

  // Utility function to save text file
  const saveTextFile = (content, filename) => {
    const blob = new Blob([content], { type: "text/plain" });
    const url = URL.createObjectURL(blob);
    const a = document.createElement("a");
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
  };

  const handleOptimize = async (code) => {
    setLoading(true);
    try {
      const res = await fetch("https://neurofold-j01j.onrender.com/run-llm", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ code }),
      });

      if (!res.ok) {
        throw new Error(`Server error: ${res.status}`);
      }

      const data = await res.json();

      // Update state
      setOptimizedCode(data.optimized_code || "");
      setOptimizationLog(data.suggestions || []);
      setStatus(data.status || "");
      setFullText(data.full_text || "");
      setUnOptimizedCode(data.unoptimized_code || "");

    } catch (err) {
      console.error("Error:", err);
      setOptimizedCode("// Error contacting backend");
      setOptimizationLog([]);
      setStatus("Error");
    } finally {
      setLoading(false);
    }
  };

  // Handler to download all files
  const handleSaveAll = () => {
    saveTextFile(unOptimizedCode, "code_before_optimization.txt");
    saveTextFile(optimizedCode, "code_after_optimization.txt");
    saveTextFile(
      Array.isArray(optimizationLog) ? optimizationLog.join("\n") : optimizationLog,
      "optimization_suggestions.txt"
    );
  };

  return (
    <div className="min-h-screen bg-base-200">
      <Navbar />
      <div className="p-8 grid grid-cols-1 lg:grid-cols-2 gap-6">
        <CodeInput onOptimize={handleOptimize} />
        <OutputPanel
          optimizedCode={optimizedCode}
          optimizationLog={optimizationLog}
          status={status}
          loading={loading}
          unOptimizedCode={unOptimizedCode}
        />
      </div>

      {/* Save All Files Button */}
      <div className="p-8">
        <button
          onClick={handleSaveAll}
          className="btn btn-primary"
          disabled={loading || (!optimizedCode && !unOptimizedCode)}
        >
          Save All Files
        </button>
      </div>
    </div>
  );
}

export default App;
