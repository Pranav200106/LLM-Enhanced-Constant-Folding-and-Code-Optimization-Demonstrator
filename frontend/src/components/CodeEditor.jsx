import { useState, useRef, useEffect } from "react";

export default function CodeEditor({ code = "", setCode }) {
  const [highlighted, setHighlighted] = useState("");
  const containerRef = useRef(null);
  const textareaRef = useRef(null);
  const gutterRef = useRef(null);
  const preRef = useRef(null);

  const highlightCode = (code) => {
    if (!code) return "";
    
    // First escape HTML
    let html = code
      .replace(/&/g, "&amp;")
      .replace(/</g, "&lt;")
      .replace(/>/g, "&gt;");

    // Apply syntax highlighting in order (use negative lookahead to avoid matching inside tags)
    
    // 1. Comments first (to prevent highlighting keywords inside comments)
    html = html.replace(
      /(\/\/[^\n]*|\/\*[\s\S]*?\*\/)/g,
      '##COMMENT_START##$1##COMMENT_END##'
    );

    // 2. Strings (to prevent highlighting keywords inside strings)
    html = html.replace(
      /("[^"]*"|'[^']*')/g,
      '##STRING_START##$1##STRING_END##'
    );

    // 3. Preprocessor directives
    html = html.replace(
      /(#\s*(?:include|define|ifdef|ifndef|endif|pragma|if|else|elif|undef)\b[^\n]*)/g,
      '##PREPROCESSOR_START##$1##PREPROCESSOR_END##'
    );

    // 4. Keywords (won't match inside protected sections)
    html = html.replace(
      /\b(int|float|char|double|return|if|else|for|while|void|break|continue|struct|class|const|unsigned|signed|long|short|sizeof|typedef|enum|union|static|extern|auto|register|volatile|inline)\b/g,
      '##KEYWORD_START##$1##KEYWORD_END##'
    );

    // 5. Numbers (won't match inside protected sections)
    html = html.replace(/\b(\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)\b/g, '##NUMBER_START##$1##NUMBER_END##');

    // Now replace placeholders with actual HTML spans
    html = html.replace(/##COMMENT_START##(.*?)##COMMENT_END##/g, '<span style="color: #6A9955; font-style: italic;">$1</span>');
    html = html.replace(/##STRING_START##(.*?)##STRING_END##/g, '<span style="color: #CE9178;">$1</span>');
    html = html.replace(/##PREPROCESSOR_START##(.*?)##PREPROCESSOR_END##/g, '<span style="color: #C586C0;">$1</span>');
    html = html.replace(/##KEYWORD_START##(.*?)##KEYWORD_END##/g, '<span style="color: #569CD6; font-weight: 600;">$1</span>');
    html = html.replace(/##NUMBER_START##(.*?)##NUMBER_END##/g, '<span style="color: #B5CEA8;">$1</span>');

    return html;
  };

  // Update highlight when code changes
  useEffect(() => {
    setHighlighted(highlightCode(code));
  }, [code]);

  // Auto-resize to fit content
  useEffect(() => {
    if (!textareaRef.current || !containerRef.current) return;
    
    const lineCount = code.split('\n').length;
    const lineHeight = 24; // 1.5rem = 24px
    const padding = 24; // 12px top + 12px bottom
    const newHeight = Math.max(384, (lineCount * lineHeight) + padding); // min 384px (24rem)
    
    containerRef.current.style.height = newHeight + 'px';
  }, [code]);

  // Handle typing logic
  const handleInput = (e) => setCode(e.target.value);

  const handleKeyDown = (e) => {
    const { selectionStart, selectionEnd, value } = e.target;

    if (e.key === "Tab") {
      e.preventDefault();
      const newValue =
        value.substring(0, selectionStart) + "  " + value.substring(selectionEnd);
      setCode(newValue);
      requestAnimationFrame(() => {
        e.target.selectionStart = e.target.selectionEnd = selectionStart + 2;
      });
    } else if (e.key === "Enter") {
      e.preventDefault();
      const lineStart = value.lastIndexOf("\n", selectionStart - 1) + 1;
      const currentLine = value.substring(lineStart, selectionStart);
      const indentMatch = currentLine.match(/^\s+/);
      const indent = indentMatch ? indentMatch[0] : "";
      const newValue =
        value.substring(0, selectionStart) +
        "\n" +
        indent +
        value.substring(selectionEnd);
      setCode(newValue);
      requestAnimationFrame(() => {
        e.target.selectionStart = e.target.selectionEnd =
          selectionStart + 1 + indent.length;
      });
    }
  };

  // Auto update gutter line numbers
  const updateLineNumbers = () => {
    if (!textareaRef.current || !gutterRef.current) return;
    const lineCount = textareaRef.current.value.split("\n").length;
    gutterRef.current.innerHTML = Array.from(
      { length: lineCount },
      (_, i) => i + 1
    ).join("<br>");
  };

  useEffect(() => {
    updateLineNumbers();
  }, [code]);

  return (
    <div 
      ref={containerRef}
      className="relative w-full bg-[#1e1e1e] text-gray-100 rounded-lg overflow-hidden shadow-lg"
      style={{ height: '384px' }}
    >
      {/* Line Numbers */}
      <div
        ref={gutterRef}
        className="absolute top-0 left-0 w-10 text-right text-gray-500 font-mono text-sm pt-3 pr-2 select-none"
        style={{
          lineHeight: "1.5rem",
          userSelect: "none",
        }}
      ></div>

      {/* Highlight Layer */}
      <pre
        ref={preRef}
        className="absolute top-0 left-0 w-full font-mono text-sm whitespace-pre-wrap p-3 pointer-events-none"
        dangerouslySetInnerHTML={{ __html: highlighted + "<br />" }}
        style={{
          marginLeft: "2.5rem",
          lineHeight: "1.5rem",
        }}
      ></pre>

      {/* Textarea (editable) */}
      <textarea
        ref={textareaRef}
        value={code}
        onChange={handleInput}
        onKeyDown={handleKeyDown}
        spellCheck={false}
        autoComplete="off"
        autoCorrect="off"
        autoCapitalize="off"
        className="absolute top-0 left-0 w-full h-full font-mono text-sm bg-transparent text-transparent caret-[#00ff88] p-3 focus:outline-none resize-none overflow-hidden"
        style={{
          marginLeft: "2.5rem",
          whiteSpace: "pre",
          tabSize: 2,
          lineHeight: "1.5rem",
        }}
        placeholder="// Write your C/C++ code here..."
      />
    </div>
  );
}