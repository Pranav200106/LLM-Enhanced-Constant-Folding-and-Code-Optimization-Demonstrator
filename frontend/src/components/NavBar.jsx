function Navbar() {
  return (
    <div className="navbar bg-base-100 shadow-md px-6 backdrop-blur-3xl">
      <div className="flex-1">
        <h1 className="text-xl font-bold text-primary">NeuroFold</h1>
      </div>
      <div className="flex-none">
        <button
          className="btn btn-accent btn-ghost"
          onClick={() => {
            const theme =
              document.documentElement.getAttribute("data-theme") === "latenotyet"
                ? "corporate"
                : "latenotyet";
            document.documentElement.setAttribute("data-theme", theme);
          }}
        >
          🌗 Toggle Theme
        </button>
      </div>
    </div>
  );
}
export default Navbar;
