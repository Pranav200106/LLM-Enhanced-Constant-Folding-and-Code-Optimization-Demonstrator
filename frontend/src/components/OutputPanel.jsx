import { useState } from "react";
import TACCostComparison from "./TACCostComparison";

function OutputPanel({
  optimizedCode,
  optimizationLog,
  status,
  loading,
  unOptimizedCode,
}) {
  const [activeTab, setActiveTab] = useState("before");

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
      {/* Scrollable Tab Headers */}
      <div className="w-full overflow-x-auto">
        <div className="tabs tabs-border flex-nowrap whitespace-nowrap">
          <a
            className={`tab tab-bordered ${
              activeTab === "before" ? "tab-active" : ""
            }`}
            onClick={() => setActiveTab("before")}
          >
            Optimized Code before LLM
          </a>
          <a
            className={`tab tab-bordered ${
              activeTab === "after" ? "tab-active" : ""
            }`}
            onClick={() => setActiveTab("after")}
          >
            Optimized Code after LLM
          </a>
          <a
            className={`tab tab-bordered ${
              activeTab === "summary" ? "tab-active" : ""
            }`}
            onClick={() => setActiveTab("summary")}
          >
            Optimization Summary
          </a>
          <a
            className={`tab tab-bordered ${
              activeTab === "performance" ? "tab-active" : ""
            }`}
            onClick={() => setActiveTab("performance")}
          >
            Performance Analysis
          </a>
        </div>
      </div>

      {/* Tab Content */}
      <div className="border-t border-base-300 mt-4">
        {activeTab === "before" && (
          <div className="p-6">
            <h2 className="text-lg text-warning font-semibold mb-2">
              Optimized Code before LLM
            </h2>
            <pre className="bg-base-200 p-3 rounded-lg font-mono whitespace-pre-wrap">
              {unOptimizedCode ||
                "// Optimized code before LLM Validation appears here"}
            </pre>
          </div>
        )}
        

        {activeTab === "after" && (
          <div className="p-6">
            <h2 className="text-lg text-success font-semibold mb-2">
              Optimized Code after LLM
            </h2>
            <pre className="bg-base-200 p-3 rounded-lg font-mono whitespace-pre-wrap">
              {optimizedCode ||
                "// Optimized code after LLM validation will appear here"}
            </pre>
          </div>
        )}

        {activeTab === "summary" && (
          <div className="p-6">
            <h3 className="text-lg font-semibold text-info mb-2">
              Optimization Summary
            </h3>
            {optimizationLog?.length > 0 ? (
              <ul>
                {optimizationLog.map((step, idx) => (
                  <div
                    key={idx}
                    className="card bg-base-200 w-auto shadow-sm mt-4"
                  >
                    <div className="card-body">
                      <p>{step}</p>
                    </div>
                  </div>
                ))}
              </ul>
            ) : (
              <p className="text-gray-500">
                No optimization suggestions available.
              </p>
            )}
          </div>
        )}

        {activeTab === "performance" && (
          <div className="p-6">
            {unOptimizedCode && optimizedCode ? (
              <TACCostComparison
                unOptimizedCode={unOptimizedCode}
                optimizedCode={optimizedCode}
              />
            ) : (
              <div className="alert alert-warning">
                <svg
                  xmlns="http://www.w3.org/2000/svg"
                  className="stroke-current shrink-0 h-6 w-6"
                  fill="none"
                  viewBox="0 0 24 24"
                >
                  <path
                    strokeLinecap="round"
                    strokeLinejoin="round"
                    strokeWidth="2"
                    d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z"
                  />
                </svg>
                <span>
                  No optimization data available. Run the optimizer to see
                  performance analysis.
                </span>
              </div>
            )}
          </div>
        )}
      </div>

      {/* Status Alerts */}
      {status === "Optimization Correct" && (
        <div role="alert" className="alert alert-success mt-4">
          <svg
            xmlns="http://www.w3.org/2000/svg"
            className="h-6 w-6 shrink-0 stroke-current"
            fill="none"
            viewBox="0 0 24 24"
          >
            <path
              strokeLinecap="round"
              strokeLinejoin="round"
              strokeWidth="2"
              d="M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z"
            />
          </svg>
          <span>{status}</span>
        </div>
      )}

      {status === "Issues Found" && (
        <div role="alert" className="alert alert-error mt-4">
          <svg
            xmlns="http://www.w3.org/2000/svg"
            className="h-6 w-6 shrink-0 stroke-current"
            fill="none"
            viewBox="0 0 24 24"
          >
            <path
              strokeLinecap="round"
              strokeLinejoin="round"
              strokeWidth="2"
              d="M12 9v2m0 4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z"
            />
          </svg>
          <span>{status}</span>
        </div>
      )}
    </div>
  );
}

export default OutputPanel;