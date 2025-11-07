// frontend/src/components/TACCostComparison.jsx
import React, { useState, useEffect } from 'react';
import { LineChart, Line, BarChart, Bar, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer } from 'recharts';
import { TrendingDown, Code, Zap, Activity } from 'lucide-react';

const TACCostComparison = ({ unOptimizedCode, optimizedCode }) => {
  const [stats, setStats] = useState(null);
  const [waveformData, setWaveformData] = useState([]);
  const [totalCostData, setTotalCostData] = useState([]);
  const [instructionCountData, setInstructionCountData] = useState([]);
  const [avgCostData, setAvgCostData] = useState([]);

  const calculateInstructionCost = (line) => {
    const lineLower = line.toLowerCase().trim();
    
    // High cost operations
    if (lineLower.includes('*') || lineLower.includes('mult')) {
      return { cost: 10, type: 'multiplication' };
    } else if (lineLower.includes('/') || lineLower.includes('div')) {
      return { cost: 10, type: 'division' };
    } else if (lineLower.includes('%') || lineLower.includes('mod')) {
      return { cost: 9, type: 'modulo' };
    }
    
    // Medium cost operations
    else if (lineLower.includes('+') && !lineLower.includes('++')) {
      return { cost: 5, type: 'addition' };
    } else if (lineLower.includes('-') && !lineLower.includes('--') && !lineLower.includes('->')) {
      return { cost: 5, type: 'subtraction' };
    }
    
    // Low cost operations
    else if (lineLower.includes('=') && !lineLower.includes('==') && !lineLower.includes('!=')) {
      return { cost: 3, type: 'assignment' };
    } else if (lineLower.includes('return')) {
      return { cost: 2, type: 'return' };
    } else if (lineLower.match(/==|!=|<|>|<=|>=/)) {
      return { cost: 4, type: 'comparison' };
    } else if (lineLower.match(/\bif\b|\bgoto\b|\blabel\b/)) {
      return { cost: 2, type: 'control_flow' };
    }
    
    // Default cost
    else {
      return { cost: 1, type: 'other' };
    }
  };

  const analyzeTACCode = (code) => {
    if (!code || typeof code !== 'string') return { costs: [], lines: [] };
    
    const lines = code.split('\n').filter(line => line.trim());
    const costs = lines.map(line => calculateInstructionCost(line).cost);
    
    return { costs, lines };
  };

  useEffect(() => {
    const irAnalysis = analyzeTACCode(unOptimizedCode);
    const optimizedAnalysis = analyzeTACCode(optimizedCode);

    // Calculate statistics
    const irTotal = irAnalysis.costs.reduce((a, b) => a + b, 0);
    const optimizedTotal = optimizedAnalysis.costs.reduce((a, b) => a + b, 0);

    const irAvg = irTotal / irAnalysis.costs.length || 0;
    const optimizedAvg = optimizedTotal / optimizedAnalysis.costs.length || 0;

    const costReduction = ((irTotal - optimizedTotal) / irTotal * 100) || 0;
    const instructionReduction = ((irAnalysis.costs.length - optimizedAnalysis.costs.length) / irAnalysis.costs.length * 100) || 0;

    setStats({
      ir: {
        instructions: irAnalysis.costs.length,
        totalCost: irTotal,
        avgCost: irAvg
      },
      optimized: {
        instructions: optimizedAnalysis.costs.length,
        totalCost: optimizedTotal,
        avgCost: optimizedAvg
      },
      improvements: {
        costReduction,
        instructionReduction
      }
    });

    // Prepare waveform data
    const maxLength = Math.max(irAnalysis.costs.length, optimizedAnalysis.costs.length);

    const waveform = [];
    for (let i = 0; i < maxLength; i++) {
      waveform.push({
        index: i,
        original: irAnalysis.costs[i] || null,
        optimized: optimizedAnalysis.costs[i] || null
      });
    }
    setWaveformData(waveform);

    // Prepare total cost data
    setTotalCostData([
      { name: 'Before LLM', cost: irTotal, fill: '#ef4444' },
      { name: 'After LLM', cost: optimizedTotal, fill: '#10b981' }
    ]);

    // Prepare instruction count data
    setInstructionCountData([
      { name: 'Before LLM', count: irAnalysis.costs.length, fill: '#ef4444' },
      { name: 'After LLM', count: optimizedAnalysis.costs.length, fill: '#10b981' }
    ]);

    // Prepare average cost data
    setAvgCostData([
      { name: 'Before LLM', avg: parseFloat(irAvg.toFixed(2)), fill: '#ef4444' },
      { name: 'After LLM', avg: parseFloat(optimizedAvg.toFixed(2)), fill: '#10b981' }
    ]);

  }, [unOptimizedCode, optimizedCode]);

  if (!stats) {
    return (
      <div className="flex items-center justify-center h-64">
        <div className="text-center">
          <span className="loading loading-spinner loading-lg text-primary"></span>
          <p className="mt-4 text-base-content">Analyzing TAC instructions...</p>
        </div>
      </div>
    );
  }

  return (
    <div className="w-full space-y-6">
      {/* Header */}
      <div className="text-center mb-6">
        <h2 className="text-3xl font-bold mb-2">TAC Instruction Cost Analysis</h2>
        <p className="text-base-content/70">Performance comparison before and after LLM optimization</p>
      </div>

      {/* Summary Cards */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-4 mb-6">
        <div className="card bg-base-100 shadow-xl border-l-4 border-error">
          <div className="card-body">
            <div className="flex items-center justify-between">
              <div>
                <p className="text-sm opacity-70 mb-1">Before LLM Optimization</p>
                <p className="text-3xl font-bold">{stats.ir.totalCost}</p>
                <p className="text-xs opacity-60">{stats.ir.instructions} instructions</p>
              </div>
              <Code className="w-12 h-12 opacity-30" />
            </div>
          </div>
        </div>

        <div className="card bg-base-100 shadow-xl border-l-4 border-success">
          <div className="card-body">
            <div className="flex items-center justify-between">
              <div>
                <p className="text-sm opacity-70 mb-1">After LLM Optimization</p>
                <p className="text-3xl font-bold">{stats.optimized.totalCost}</p>
                <p className="text-xs opacity-60">{stats.optimized.instructions} instructions</p>
              </div>
              <Zap className="w-12 h-12 opacity-30" />
            </div>
            <div className="mt-3 pt-3 border-t border-base-300">
              <div className="flex items-center text-success">
                <TrendingDown className="w-4 h-4 mr-1" />
                <span className="text-sm font-semibold">
                  {stats.improvements.costReduction.toFixed(1)}% cost reduction
                </span>
              </div>
            </div>
          </div>
        </div>
      </div>

      {/* Charts Grid */}
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        {/* Waveform Chart */}
        <div className="card bg-base-100 shadow-xl">
          <div className="card-body">
            <h3 className="card-title text-lg mb-4">Instruction Cost Waveform</h3>
            <ResponsiveContainer width="100%" height={300}>
              <LineChart data={waveformData}>
                <CartesianGrid strokeDasharray="3 3" stroke="#e5e7eb" />
                <XAxis 
                  dataKey="index" 
                  label={{ value: 'Instruction Index', position: 'insideBottom', offset: -5 }}
                  tick={{ fontSize: 12 }}
                />
                <YAxis 
                  label={{ value: 'Cost', angle: -90, position: 'insideLeft' }}
                  tick={{ fontSize: 12 }}
                />
                <Tooltip 
                  contentStyle={{ backgroundColor: 'rgba(255, 255, 255, 0.95)', border: '1px solid #ccc' }}
                />
                <Legend />
                <Line 
                  type="monotone" 
                  dataKey="original" 
                  stroke="#ef4444" 
                  strokeWidth={2}
                  dot={{ r: 3 }}
                  name="Before LLM"
                />
                <Line 
                  type="monotone" 
                  dataKey="optimized" 
                  stroke="#10b981" 
                  strokeWidth={2}
                  dot={{ r: 3 }}
                  name="After LLM"
                />
              </LineChart>
            </ResponsiveContainer>
          </div>
        </div>

        {/* Total Cost Bar Chart */}
        <div className="card bg-base-100 shadow-xl">
          <div className="card-body">
            <h3 className="card-title text-lg mb-4">Total Instruction Cost</h3>
            <ResponsiveContainer width="100%" height={300}>
              <BarChart data={totalCostData}>
                <CartesianGrid strokeDasharray="3 3" stroke="#e5e7eb" />
                <XAxis dataKey="name" tick={{ fontSize: 12 }} />
                <YAxis tick={{ fontSize: 12 }} />
                <Tooltip 
                  contentStyle={{ backgroundColor: 'rgba(255, 255, 255, 0.95)', border: '1px solid #ccc' }}
                />
                <Bar dataKey="cost" radius={[8, 8, 0, 0]}>
                  {totalCostData.map((entry, index) => (
                    <rect key={`cell-${index}`} fill={entry.fill} />
                  ))}
                </Bar>
              </BarChart>
            </ResponsiveContainer>
          </div>
        </div>

        {/* Instruction Count Chart */}
        <div className="card bg-base-100 shadow-xl">
          <div className="card-body">
            <h3 className="card-title text-lg mb-4">Instruction Count</h3>
            <ResponsiveContainer width="100%" height={300}>
              <BarChart data={instructionCountData}>
                <CartesianGrid strokeDasharray="3 3" stroke="#e5e7eb" />
                <XAxis dataKey="name" tick={{ fontSize: 12 }} />
                <YAxis tick={{ fontSize: 12 }} />
                <Tooltip 
                  contentStyle={{ backgroundColor: 'rgba(255, 255, 255, 0.95)', border: '1px solid #ccc' }}
                />
                <Bar dataKey="count" radius={[8, 8, 0, 0]}>
                  {instructionCountData.map((entry, index) => (
                    <rect key={`cell-${index}`} fill={entry.fill} />
                  ))}
                </Bar>
              </BarChart>
            </ResponsiveContainer>
            <div className="text-center mt-4">
              <span className="badge badge-success badge-lg">
                {stats.improvements.instructionReduction.toFixed(1)}% fewer instructions
              </span>
            </div>
          </div>
        </div>

        {/* Average Cost Chart */}
        <div className="card bg-base-100 shadow-xl">
          <div className="card-body">
            <h3 className="card-title text-lg mb-4">Average Cost per Instruction</h3>
            <ResponsiveContainer width="100%" height={300}>
              <BarChart data={avgCostData}>
                <CartesianGrid strokeDasharray="3 3" stroke="#e5e7eb" />
                <XAxis dataKey="name" tick={{ fontSize: 12 }} />
                <YAxis tick={{ fontSize: 12 }} />
                <Tooltip 
                  contentStyle={{ backgroundColor: 'rgba(255, 255, 255, 0.95)', border: '1px solid #ccc' }}
                />
                <Bar dataKey="avg" radius={[8, 8, 0, 0]}>
                  {avgCostData.map((entry, index) => (
                    <rect key={`cell-${index}`} fill={entry.fill} />
                  ))}
                </Bar>
              </BarChart>
            </ResponsiveContainer>
          </div>
        </div>
      </div>

      {/* Detailed Statistics */}
      <div className="card bg-base-100 shadow-xl">
        <div className="card-body">
          <h3 className="card-title text-lg mb-4">Detailed Statistics</h3>
          <div className="overflow-x-auto">
            <table className="table table-zebra">
              <thead>
                <tr>
                  <th>Metric</th>
                  <th className="text-right">Before LLM</th>
                  <th className="text-right">After LLM</th>
                  <th className="text-right">Improvement</th>
                </tr>
              </thead>
              <tbody>
                <tr>
                  <td className="font-medium">Instructions</td>
                  <td className="text-right">{stats.ir.instructions}</td>
                  <td className="text-right">{stats.optimized.instructions}</td>
                  <td className="text-right text-success font-semibold">
                    -{stats.ir.instructions - stats.optimized.instructions}
                  </td>
                </tr>
                <tr>
                  <td className="font-medium">Total Cost</td>
                  <td className="text-right">{stats.ir.totalCost}</td>
                  <td className="text-right">{stats.optimized.totalCost}</td>
                  <td className="text-right text-success font-semibold">
                    -{stats.improvements.costReduction.toFixed(1)}%
                  </td>
                </tr>
                <tr>
                  <td className="font-medium">Avg Cost/Instruction</td>
                  <td className="text-right">{stats.ir.avgCost.toFixed(2)}</td>
                  <td className="text-right">{stats.optimized.avgCost.toFixed(2)}</td>
                  <td className="text-right text-success font-semibold">
                    {((stats.ir.avgCost - stats.optimized.avgCost) / stats.ir.avgCost * 100).toFixed(1)}%
                  </td>
                </tr>
              </tbody>
            </table>
          </div>
        </div>
      </div>
    </div>
  );
};

export default TACCostComparison;