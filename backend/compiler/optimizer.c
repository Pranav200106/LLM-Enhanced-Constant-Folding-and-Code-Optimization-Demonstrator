#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// ==================== IR INSTRUCTION TYPES ====================

typedef enum {
    IR_ASSIGN,          // t1 = 5 or result = t1
    IR_ADD,             // t1 = t2 + t3
    IR_SUB,             // t1 = t2 - t3
    IR_MUL,             // t1 = t2 * t3
    IR_DIV,             // t1 = t2 / t3
    IR_MOD,             // t1 = t2 % t3
    IR_LT,              // t1 = t2 < t3
    IR_LE,              // t1 = t2 <= t3
    IR_GT,              // t1 = t2 > t3
    IR_GE,              // t1 = t2 >= t3
    IR_EQ,              // t1 = t2 == t3
    IR_NE,              // t1 = t2 != t3
    IR_UMINUS,          // t1 = -t2
    IR_LABEL,           // L1:
    IR_GOTO,            // GOTO L1
    IR_IF_FALSE,        // IF_FALSE t1 GOTO L1
    IR_PARAM,           // PARAM n or PUSH_PARAM t1
    IR_CALL,            // t1 = CALL func, 1
    IR_RETURN,          // RETURN t1
    IR_FUNCTION,        // FUNCTION main:
    IR_END_FUNCTION,    // END FUNCTION
    IR_DECLARE,         // DECLARE x
    IR_COMMENT,         // Comment or empty line
    IR_UNKNOWN
} IROpcode;

typedef struct {
    IROpcode opcode;
    char result[64];
    char arg1[64];
    char arg2[64];
    char label[64];
    char originalLine[256];
    bool isConstant;
    int constantValue;
    bool isOptimized;
} IRInstruction;

typedef struct {
    IRInstruction* instructions;
    int count;
    int capacity;
} IRCode;

// ==================== CONSTANT TABLE ====================

typedef struct {
    char var[64];
    int value;
    bool isConstant;
} ConstantEntry;

typedef struct {
    ConstantEntry entries[512];
    int count;
} ConstantTable;

ConstantTable constTable;

void initConstantTable() {
    constTable.count = 0;
}

void addConstant(const char* var, int value) {
    for (int i = 0; i < constTable.count; i++) {
        if (strcmp(constTable.entries[i].var, var) == 0) {
            constTable.entries[i].value = value;
            constTable.entries[i].isConstant = true;
            return;
        }
    }
    strcpy(constTable.entries[constTable.count].var, var);
    constTable.entries[constTable.count].value = value;
    constTable.entries[constTable.count].isConstant = true;
    constTable.count++;
}

void removeConstant(const char* var) {
    for (int i = 0; i < constTable.count; i++) {
        if (strcmp(constTable.entries[i].var, var) == 0) {
            constTable.entries[i].isConstant = false;
            return;
        }
    }
}

bool getConstant(const char* var, int* value) {
    for (int i = 0; i < constTable.count; i++) {
        if (strcmp(constTable.entries[i].var, var) == 0 && 
            constTable.entries[i].isConstant) {
            *value = constTable.entries[i].value;
            return true;
        }
    }
    return false;
}

bool isNumber(const char* str) {
    if (!str || !*str) return false;
    if (*str == '-' || *str == '+') str++;
    if (!*str) return false;
    while (*str) {
        if (!isdigit(*str)) return false;
        str++;
    }
    return true;
}

int toNumber(const char* str) {
    return atoi(str);
}

// ==================== IR CODE MANAGEMENT ====================

void initIRCode(IRCode* ir) {
    ir->capacity = 200;
    ir->count = 0;
    ir->instructions = (IRInstruction*)malloc(sizeof(IRInstruction) * ir->capacity);
}

void addInstruction(IRCode* ir, IRInstruction instr) {
    if (ir->count >= ir->capacity) {
        ir->capacity *= 2;
        ir->instructions = (IRInstruction*)realloc(ir->instructions, 
                                                   sizeof(IRInstruction) * ir->capacity);
    }
    ir->instructions[ir->count++] = instr;
}

// ==================== STRING UTILITIES ====================

char* trim(char* str) {
    while (isspace(*str)) str++;
    if (*str == 0) return str;
    char* end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    *(end + 1) = 0;
    return str;
}

// ==================== IR PARSER ====================

IRInstruction parseIRLine(const char* line) {
    IRInstruction instr;
    memset(&instr, 0, sizeof(IRInstruction));
    instr.isConstant = false;
    instr.isOptimized = false;
    strcpy(instr.originalLine, line);
    
    char buffer[256];
    strcpy(buffer, line);
    char* ptr = trim(buffer);
    
    // Empty line or comment
    if (strlen(ptr) == 0) {
        instr.opcode = IR_COMMENT;
        return instr;
    }
    
    // Check for label (ends with :)
    if (strchr(ptr, ':') && ptr[strlen(ptr) - 1] == ':') {
        instr.opcode = IR_LABEL;
        sscanf(ptr, "%[^:]", instr.label);
        return instr;
    }
    
    // Check for FUNCTION
    if (strncmp(ptr, "FUNCTION", 8) == 0) {
        instr.opcode = IR_FUNCTION;
        sscanf(ptr, "FUNCTION %[^:]", instr.label);
        return instr;
    }
    
    // Check for END FUNCTION
    if (strncmp(ptr, "END FUNCTION", 12) == 0) {
        instr.opcode = IR_END_FUNCTION;
        sscanf(ptr + 13, "%s", instr.label);
        return instr;
    }
    
    // Check for DECLARE
    if (strncmp(ptr, "DECLARE", 7) == 0) {
        instr.opcode = IR_DECLARE;
        sscanf(ptr, "DECLARE %s", instr.result);
        return instr;
    }
    
    // Check for PARAM (both styles)
    if (strncmp(ptr, "PARAM", 5) == 0) {
        instr.opcode = IR_PARAM;
        sscanf(ptr, "PARAM %s", instr.arg1);
        return instr;
    }
    
    // Check for PUSH_PARAM
    if (strncmp(ptr, "PUSH_PARAM", 10) == 0) {
        instr.opcode = IR_PARAM;
        sscanf(ptr, "PUSH_PARAM %s", instr.arg1);
        return instr;
    }
    
    // Check for GOTO
    if (strncmp(ptr, "GOTO", 4) == 0) {
        instr.opcode = IR_GOTO;
        sscanf(ptr, "GOTO %s", instr.label);
        return instr;
    }
    
    // Check for IF_FALSE
    if (strncmp(ptr, "IF_FALSE", 8) == 0) {
        instr.opcode = IR_IF_FALSE;
        sscanf(ptr, "IF_FALSE %s GOTO %s", instr.arg1, instr.label);
        return instr;
    }
    
    // Check for RETURN
    if (strncmp(ptr, "RETURN", 6) == 0) {
        instr.opcode = IR_RETURN;
        char* retVal = strchr(ptr, ' ');
        if (retVal) {
            sscanf(retVal, "%s", instr.arg1);
        }
        return instr;
    }
    
    // Check for CALL
    if (strstr(ptr, "CALL")) {
        instr.opcode = IR_CALL;
        char* equalSign = strchr(ptr, '=');
        if (equalSign) {
            *equalSign = '\0';
            sscanf(ptr, "%s", instr.result);
            sscanf(equalSign + 1, "CALL %[^,]", instr.arg1);
        }
        return instr;
    }
    
    // Assignment or binary operation
    char* equals = strchr(ptr, '=');
    if (equals) {
        *equals = '\0';
        sscanf(ptr, "%s", instr.result);
        ptr = equals + 1;
        ptr = trim(ptr);
        
        // Check for unary minus
        if (*ptr == '-' && !strchr(ptr + 1, ' ')) {
            instr.opcode = IR_UMINUS;
            sscanf(ptr + 1, "%s", instr.arg1);
            return instr;
        }
        
        // Check for binary operations
        char* op = NULL;
        IROpcode opcode = IR_UNKNOWN;
        
        if ((op = strstr(ptr, "<="))) {
            opcode = IR_LE;
        } else if ((op = strstr(ptr, ">="))) {
            opcode = IR_GE;
        } else if ((op = strstr(ptr, "=="))) {
            opcode = IR_EQ;
        } else if ((op = strstr(ptr, "!="))) {
            opcode = IR_NE;
        } else if ((op = strchr(ptr, '<'))) {
            opcode = IR_LT;
        } else if ((op = strchr(ptr, '>'))) {
            opcode = IR_GT;
        } else if ((op = strchr(ptr, '+'))) {
            opcode = IR_ADD;
        } else if ((op = strchr(ptr, '-'))) {
            opcode = IR_SUB;
        } else if ((op = strchr(ptr, '*'))) {
            opcode = IR_MUL;
        } else if ((op = strchr(ptr, '/'))) {
            opcode = IR_DIV;
        } else if ((op = strchr(ptr, '%'))) {
            opcode = IR_MOD;
        }
        
        if (op && opcode != IR_UNKNOWN) {
            instr.opcode = opcode;
            *op = '\0';
            sscanf(trim(ptr), "%s", instr.arg1);
            // Skip the operator (might be 2 chars like <=)
            char* arg2Start = op + 1;
            if (*arg2Start == '=' || *arg2Start == '<' || *arg2Start == '>') {
                arg2Start++;
            }
            sscanf(trim(arg2Start), "%s", instr.arg2);
        } else {
            // Simple assignment
            instr.opcode = IR_ASSIGN;
            sscanf(ptr, "%s", instr.arg1);
        }
    } else {
        instr.opcode = IR_UNKNOWN;
    }
    
    return instr;
}

// ==================== CONSTANT FOLDING OPTIMIZATION ====================

bool performConstantFolding(IRInstruction* instr) {
    int val1, val2, result;
    bool isConst1, isConst2;
    
    switch (instr->opcode) {
        case IR_ASSIGN:
            // t1 = 5 or t1 = t2
            if (isNumber(instr->arg1)) {
                instr->isConstant = true;
                instr->constantValue = toNumber(instr->arg1);
                addConstant(instr->result, instr->constantValue);
                return true;
            } else if (getConstant(instr->arg1, &val1)) {
                instr->isConstant = true;
                instr->constantValue = val1;
                sprintf(instr->arg1, "%d", val1);
                addConstant(instr->result, val1);
                instr->isOptimized = true;
                return true;
            } else {
                removeConstant(instr->result);
            }
            break;
            
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
        case IR_MOD:
        case IR_LT:
        case IR_LE:
        case IR_GT:
        case IR_GE:
        case IR_EQ:
        case IR_NE:
            isConst1 = isNumber(instr->arg1) || getConstant(instr->arg1, &val1);
            isConst2 = isNumber(instr->arg2) || getConstant(instr->arg2, &val2);
            
            if (!isConst1 && isNumber(instr->arg1)) {
                val1 = toNumber(instr->arg1);
                isConst1 = true;
            }
            if (!isConst2 && isNumber(instr->arg2)) {
                val2 = toNumber(instr->arg2);
                isConst2 = true;
            }
            
            if (isConst1 && isConst2) {
                switch (instr->opcode) {
                    case IR_ADD: result = val1 + val2; break;
                    case IR_SUB: result = val1 - val2; break;
                    case IR_MUL: result = val1 * val2; break;
                    case IR_DIV: 
                        if (val2 == 0) {
                            removeConstant(instr->result);
                            return false;
                        }
                        result = val1 / val2; 
                        break;
                    case IR_MOD: 
                        if (val2 == 0) {
                            removeConstant(instr->result);
                            return false;
                        }
                        result = val1 % val2; 
                        break;
                    case IR_LT: result = (val1 < val2) ? 1 : 0; break;
                    case IR_LE: result = (val1 <= val2) ? 1 : 0; break;
                    case IR_GT: result = (val1 > val2) ? 1 : 0; break;
                    case IR_GE: result = (val1 >= val2) ? 1 : 0; break;
                    case IR_EQ: result = (val1 == val2) ? 1 : 0; break;
                    case IR_NE: result = (val1 != val2) ? 1 : 0; break;
                    default: return false;
                }
                
                // Replace with constant assignment
                instr->opcode = IR_ASSIGN;
                sprintf(instr->arg1, "%d", result);
                instr->arg2[0] = '\0';
                instr->isConstant = true;
                instr->constantValue = result;
                instr->isOptimized = true;
                addConstant(instr->result, result);
                return true;
            } else {
                // Propagate constants in operands
                bool changed = false;
                if (isConst1 && !isNumber(instr->arg1)) {
                    sprintf(instr->arg1, "%d", val1);
                    changed = true;
                }
                if (isConst2 && !isNumber(instr->arg2)) {
                    sprintf(instr->arg2, "%d", val2);
                    changed = true;
                }
                removeConstant(instr->result);
                return changed;
            }
            break;
            
        case IR_UMINUS:
            if (isNumber(instr->arg1)) {
                val1 = toNumber(instr->arg1);
                result = -val1;
                instr->opcode = IR_ASSIGN;
                sprintf(instr->arg1, "%d", result);
                instr->isConstant = true;
                instr->constantValue = result;
                instr->isOptimized = true;
                addConstant(instr->result, result);
                return true;
            } else if (getConstant(instr->arg1, &val1)) {
                result = -val1;
                instr->opcode = IR_ASSIGN;
                sprintf(instr->arg1, "%d", result);
                instr->isConstant = true;
                instr->constantValue = result;
                instr->isOptimized = true;
                addConstant(instr->result, result);
                return true;
            } else {
                removeConstant(instr->result);
            }
            break;
            
        case IR_CALL:
            // Function calls invalidate constants in result
            removeConstant(instr->result);
            break;
            
        case IR_PARAM:
            // Propagate constant parameters
            if (getConstant(instr->arg1, &val1)) {
                sprintf(instr->arg1, "%d", val1);
                return true;
            }
            break;
            
        case IR_IF_FALSE:
            // Propagate constant in condition
            if (getConstant(instr->arg1, &val1)) {
                sprintf(instr->arg1, "%d", val1);
                return true;
            }
            break;
            
        case IR_RETURN:
            // Propagate constant in return value
            if (strlen(instr->arg1) > 0 && getConstant(instr->arg1, &val1)) {
                sprintf(instr->arg1, "%d", val1);
                return true;
            }
            break;
            
        default:
            break;
    }
    
    return false;
}

void optimizeIRCode(IRCode* ir) {
    int optimizationsMade = 0;
    
    for (int i = 0; i < ir->count; i++) {
        // Reset constant table at function boundaries
        if (ir->instructions[i].opcode == IR_FUNCTION) {
            initConstantTable();
        }
        
        if (performConstantFolding(&ir->instructions[i])) {
            optimizationsMade++;
        }
    }
    
    printf("Total constant folding optimizations: %d\n", optimizationsMade);
}

// ==================== IR PRINTER ====================

void printIRInstruction(FILE* fp, IRInstruction* instr) {
    switch (instr->opcode) {
        case IR_ASSIGN:
            fprintf(fp, "  %s = %s", instr->result, instr->arg1);
            if (instr->isOptimized) {
                fprintf(fp, "    ; [OPTIMIZED]");
            }
            fprintf(fp, "\n");
            break;
            
        case IR_ADD:
            fprintf(fp, "  %s = %s + %s\n", instr->result, instr->arg1, instr->arg2);
            break;
            
        case IR_SUB:
            fprintf(fp, "  %s = %s - %s\n", instr->result, instr->arg1, instr->arg2);
            break;
            
        case IR_MUL:
            fprintf(fp, "  %s = %s * %s\n", instr->result, instr->arg1, instr->arg2);
            break;
            
        case IR_DIV:
            fprintf(fp, "  %s = %s / %s\n", instr->result, instr->arg1, instr->arg2);
            break;
            
        case IR_MOD:
            fprintf(fp, "  %s = %s %% %s\n", instr->result, instr->arg1, instr->arg2);
            break;
            
        case IR_LT:
            fprintf(fp, "  %s = %s < %s\n", instr->result, instr->arg1, instr->arg2);
            break;
            
        case IR_LE:
            fprintf(fp, "  %s = %s <= %s\n", instr->result, instr->arg1, instr->arg2);
            break;
            
        case IR_GT:
            fprintf(fp, "  %s = %s > %s\n", instr->result, instr->arg1, instr->arg2);
            break;
            
        case IR_GE:
            fprintf(fp, "  %s = %s >= %s\n", instr->result, instr->arg1, instr->arg2);
            break;
            
        case IR_EQ:
            fprintf(fp, "  %s = %s == %s\n", instr->result, instr->arg1, instr->arg2);
            break;
            
        case IR_NE:
            fprintf(fp, "  %s = %s != %s\n", instr->result, instr->arg1, instr->arg2);
            break;
            
        case IR_UMINUS:
            fprintf(fp, "  %s = -%s\n", instr->result, instr->arg1);
            break;
            
        case IR_LABEL:
            fprintf(fp, "%s:\n", instr->label);
            break;
            
        case IR_GOTO:
            fprintf(fp, "  GOTO %s\n", instr->label);
            break;
            
        case IR_IF_FALSE:
            fprintf(fp, "  IF_FALSE %s GOTO %s\n", instr->arg1, instr->label);
            break;
            
        case IR_PARAM:
            fprintf(fp, "  PUSH_PARAM %s\n", instr->arg1);
            break;
            
        case IR_CALL:
            fprintf(fp, "  %s = CALL %s, 1\n", instr->result, instr->arg1);
            break;
            
        case IR_RETURN:
            if (strlen(instr->arg1) > 0) {
                fprintf(fp, "  RETURN %s\n", instr->arg1);
            } else {
                fprintf(fp, "  RETURN\n");
            }
            break;
            
        case IR_FUNCTION:
            fprintf(fp, "FUNCTION %s:\n", instr->label);
            break;
            
        case IR_END_FUNCTION:
            if (strlen(instr->label) > 0) {
                fprintf(fp, "END FUNCTION %s\n\n", instr->label);
            } else {
                fprintf(fp, "END FUNCTION\n\n");
            }
            break;
            
        case IR_DECLARE:
            fprintf(fp, "  DECLARE %s\n", instr->result);
            break;
            
        case IR_COMMENT:
            fprintf(fp, "%s\n", instr->originalLine);
            break;
            
        default:
            fprintf(fp, "%s\n", instr->originalLine);
            break;
    }
}

// ==================== FILE I/O ====================

bool readIRFromFile(const char* filename, IRCode* ir) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("Error: Cannot open input file '%s'\n", filename);
        return false;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        IRInstruction instr = parseIRLine(line);
        addInstruction(ir, instr);
    }
    
    fclose(fp);
    return true;
}

bool writeIRToFile(const char* filename, IRCode* ir) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        printf("Error: Cannot create output file '%s'\n", filename);
        return false;
    }
        
    for (int i = 0; i < ir->count; i++) {
        printIRInstruction(fp, &ir->instructions[i]);
    }
    
    fclose(fp);
    return true;
}

// ==================== MAIN ====================

int main() {
    printf("=======================================================\n");
    printf("  Constant Folding Optimizer for Three-Address Code\n");
    printf("=======================================================\n\n");
    
    const char* inputFile;
    const char* outputFile;
    
    inputFile = "IR.txt";
    outputFile = "Output.txt";
    
    printf("Input file:  %s\n", inputFile);
    printf("Output file: %s\n\n", outputFile);
    
    // Initialize IR code structure
    IRCode irCode;
    initIRCode(&irCode);
    initConstantTable();
    
    // Read IR from file
    printf("Reading IR code from file...\n");
    if (!readIRFromFile(inputFile, &irCode)) {
        return 1;
    }
    printf("Successfully read %d instructions\n\n", irCode.count);
    
    // Perform constant folding optimization
    printf("Performing constant folding optimization...\n");
    initConstantTable();
    optimizeIRCode(&irCode);
    printf("\n");
    
    // Write optimized IR to file
    printf("Writing optimized IR code to file...\n");
    if (!writeIRToFile(outputFile, &irCode)) {
        free(irCode.instructions);
        return 1;
    }
    printf("Successfully wrote optimized code to '%s'\n\n", outputFile);
    
    // Statistics
    int constantFolds = 0;
    for (int i = 0; i < irCode.count; i++) {
        if (irCode.instructions[i].isOptimized) {
            constantFolds++;
        }
    }
    
    printf("=== OPTIMIZATION SUMMARY ===\n");
    printf("Total instructions: %d\n", irCode.count);
    printf("Optimized instructions: %d\n", constantFolds);
    printf("Optimization percentage: %.2f%%\n\n", 
           (float)constantFolds / irCode.count * 100);
    
    printf("Optimization completed successfully!\n");
    
    free(irCode.instructions);
    return 0;
}