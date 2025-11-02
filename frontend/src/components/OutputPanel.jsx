function OutputPanel({ optimizedCode, optimizationLog, status, loading, unOptimizedCode }) {
    if (loading) {
    return (
      <div className="flex flex-col justify-center items-center h-full bg-base-100 rounded-xl shadow-md p-6">
        <span className="loading loading-spinner loading-lg text-primary"></span>
        <p className="mt-4 text-lg font-semibold text-primary">
          Optimizing with LLM...
        </p>
      </div>
    );
  }
  return (
    
    <div className="card bg-base-100 shadow-xl p-6 w-full">
        <div className="tabs tabs-border">

            <input type="radio" name="output" className="tab" aria-label="Optimized Code before LLM" defaultChecked/>
            <div className="tab-content border-base-300 bg-base-100 p-10">
                <h2 className="text-lg text-warning font-semibold mb-2">Optimized Code before LLM</h2>
                <pre className="bg-base-200 p-3 rounded-lg font-mono whitespace-pre-wrap">
                    {unOptimizedCode|| "// Optimized code before LLM Validation appears here"}
                </pre>
            </div>

            <input type="radio" name="output" className="tab" aria-label="Optimized Code after LLM"/>
            <div className="tab-content border-base-300 bg-base-100 p-10">
                <h2 className="text-lg font-semibold mb-2 text-success">Optimized Code after LLM</h2>
                <pre className="bg-base-200 p-3 rounded-lg font-mono whitespace-pre-wrap">
                    {optimizedCode || "// Optimized code after LLM validation will appear here"}
                </pre>
            </div>

            <input type="radio" name="output" className="tab" aria-label="Optimization Summary" />
            <div className="tab-content border-base-300 bg-base-100 p-10">
                {optimizationLog && (
                    <>
                    <h3 className="text-lg font-semibold text-info mb-2">Optimization Summary</h3>
                    <ul className="list-disc">
                        {optimizationLog.map((step, idx) => (
                            <div className="card bg-base-200 w-auto shadow-sm mt-4">
                                <div className="card-body">
                                    <p>{step}</p>
                                </div>
                            </div>
                        ))}
                    </ul>
                    </>
                )}
            </div>
        </div>

      
      {status == "Optimization Correct" && (
        <div role="alert" className="alert alert-success mt-4">
        <svg xmlns="http://www.w3.org/2000/svg" className="h-6 w-6 shrink-0 stroke-current" fill="none" viewBox="0 0 24 24">
            <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z" />
        </svg>
        <span>{status}</span>
        </div>
      )}
      {status == "Issues Found" && (
        <div role="alert" className="alert alert-error mt-4">
        <svg xmlns="http://www.w3.org/2000/svg" className="h-6 w-6 shrink-0 stroke-current" fill="none" viewBox="0 0 24 24">
            <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z" />
        </svg>
        <span>{status}</span>
        </div>
      )}
    </div>
  );
}
export default OutputPanel;
