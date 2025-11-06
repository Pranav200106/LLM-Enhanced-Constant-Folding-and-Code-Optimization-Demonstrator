# NeuroFold: LLM-Enhanced Code Optimizer

**NeuroFold** is an innovative, AI-assisted compiler optimization project that combines a traditional C-based compiler with a powerful Large Language Model (LLM) to enhance constant folding and overall code optimization. The system intelligently identifies constant expressions, performs compile-time simplifications, and leverages the Gemini LLM to validate semantic correctness and suggest further safe optimizations.

## ✨ Features

- **Automated Code Optimization**: Simply paste your C code into the web interface and get an optimized version in seconds.
- **Constant Folding**: The C-based optimizer automatically identifies and evaluates constant expressions at compile time.
- **LLM-Powered Analysis**: The optimized code is reviewed by the Gemini LLM, which verifies semantic correctness, identifies potential issues (like semantic errors in factorial logic), and suggests further optimizations (e.g., aggressive constant folding, dead code elimination, strength reduction for identity operations).
- **Interactive UI**: A user-friendly web interface built with React and Tailwind CSS allows you to easily input code and view the results.
- **Detailed Feedback**: The UI displays the optimized code, the LLM's suggestions, a summary of the optimization status, and highlights any issues found, such as semantic errors or redundant assignments.

## 🚀 How It Works

The project is divided into a frontend, a backend, and a C-based compiler/optimizer. Here's a high-level overview of the workflow:

1.  **Code Submission**: The user enters C code into the React-based frontend and clicks "Optimize."
2.  **Backend Processing**: The code is sent to a Flask backend, which saves it to a `source.c` file.
3.  **Compilation**: The backend invokes a C-based compiler, which performs lexical analysis, parsing, and semantic analysis to generate an Abstract Syntax Tree (AST).
4.  **Intermediate Code Generation**: The compiler generates three-address code (TAC) from the AST and saves it to `IR.txt`.
5.  **Optimization**: A separate C-based optimizer reads `IR.txt`, performs constant folding, and writes the optimized TAC to `Output.txt`.
6.  **LLM Review**: The optimized TAC is sent to the Gemini API, which reviews the code for semantic correctness, identifies issues (e.g., semantic errors, redundant assignments), and suggests further optimizations (e.g., aggressive constant folding, dead code elimination, strength reduction).
7.  **Results Display**: The backend sends the optimized code, LLM suggestions, and status back to the frontend, where it is displayed to the user.

## 🛠️ Tech Stack

- **Frontend**: React, Vite, Tailwind CSS, daisyUI
- **Backend**: Flask, Python
- **Compiler/Optimizer**: C
- **LLM**: Google Gemini

## ⚙️ Getting Started

### Prerequisites

- Node.js and npm
- Python and pip
- A C compiler (like GCC)
- A Gemini API key

### Installation

1.  **Clone the repository**:
    ```bash
    git clone https://github.com/your-username/LLM-Enhanced-Constant-Folding-and-Code-Optimization-Demonstrator.git
    cd LLM-Enhanced-Constant-Folding-and-Code-Optimization-Demonstrator
    ```

2.  **Install frontend dependencies**:
    ```bash
    cd frontend
    npm install
    ```

3.  **Install backend dependencies**:
    ```bash
    cd ../backend
    pip install -r requirements.txt
    ```

4.  **Compile the C code**:
    ```bash
    cd compiler
    gcc -o compiler compiler.c
    gcc -o optimizer optimizer.c
    ```

5.  **Set up environment variables**:
    - Create a `.env` file in the `backend` directory.
    - Add your Gemini API key to the `.env` file:
      ```
      GEMINI_API_KEY=your_api_key_here
      ```

### Running the Application

1.  **Start the backend server**:
    ```bash
    cd backend
    python app.py
    ```

2.  **Start the frontend development server**:
    ```bash
    cd frontend
    npm run dev
    ```

3.  Open your browser and navigate to `http://localhost:5173` to use the application.

## 🤝 Contributing

Contributions are welcome! If you have any ideas, suggestions, or bug reports, please open an issue or submit a pull request.

## ⚡ Hardware Implementation (Optional)

This project also includes an optional hardware component for demonstrating code optimization on an Arduino board. The `hardware/` directory contains the necessary code for this implementation.

### Components:

- Arduino board (e.g., Arduino Uno)
- Jumper wires
- Breadboard (optional)

### Setup:

1.  Connect your Arduino board to your computer.
2.  Open the `hardware/arduino.cpp` file in the Arduino IDE.
3.  Upload the code to your Arduino board.
4.  Run the `hardware/main.py` script to interact with the Arduino and observe the optimized code execution.

## 📄 License

This project is licensed under the MIT License. See the `LICENSE` file for details.
