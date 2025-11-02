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

  const handleOptimize = async (code) => {
    setLoading(true); // Start spinner
    try {
      const res = await fetch("http://https://neurofold-j01j.onrender.com/run-llm", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ code }),
      });

      if (!res.ok) {
        throw new Error(`Server error: ${res.status}`);
      }

      const data = await res.json();
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
    </div>
  );
}

export default App;
