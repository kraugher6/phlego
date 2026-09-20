# Hazard Detection and Forwarding Implementation

## Overview
This implementation adds complete data hazard detection (RAW, WAR, WAW) with appropriate stall insertion and forwarding paths from EX/MEM and MEM/WB stages to the Phlego RISC-V emulator.

## Features Implemented

### 1. Hazard Detection
- **RAW (Read After Write) Hazards**: Detected when an instruction tries to read a register that is being written by a previous instruction
- **WAR (Write After Read) Hazards**: Detected when an instruction writes to a register that will be read by a previous instruction
- **WAW (Write After Write) Hazards**: Detected when two instructions try to write to the same register

### 2. Forwarding Paths
- **EX/MEM Forwarding**: Values from the execute stage can be forwarded directly to dependent instructions in the decode stage
- **MEM/WB Forwarding**: Values from the memory stage can be forwarded to dependent instructions in the decode stage

### 3. Stall Insertion
- **Load-Use Hazards**: Special handling for cases where an instruction depends on the result of a load instruction (which isn't available until the memory stage completes)
- **Pipeline Bubbles**: Automatic insertion of NOP cycles when hazards cannot be resolved through forwarding

## Implementation Details

### Data Structures
- `HazardType` enum: NONE, RAW, WAR, WAW
- `HazardResult` struct: Contains hazard type, stall requirement, and forwarding flags

### Methods Added
- `detect_hazard()`: Analyzes pipeline stages to identify potential hazards
- `get_forwarded_value()`: Retrieves register values from appropriate pipeline stages when forwarding is needed

### Pipeline Stage Modifications
- **Decode Stage**: Checks for hazards before moving instructions to execute stage
- **Execute Stage**: Uses forwarded values when hazards are detected
- **Memory Stage**: Maintains proper data flow for load/store operations

## How It Works

1. **Hazard Detection**: During the decode stage, the CPU checks if the current instruction has any dependencies on registers being written by previous instructions in the pipeline.

2. **Forwarding Decision**: If a RAW hazard is detected, the system determines whether the required value can be forwarded from the EX/MEM or MEM/WB stages.

3. **Stall Insertion**: For load-use hazards where forwarding isn't possible, the pipeline inserts a bubble (stall cycle) to ensure correct execution.

4. **Value Forwarding**: When forwarding is possible, the CPU retrieves the required register values directly from the appropriate pipeline stage rather than waiting for write-back.

## Test Case
The implementation handles the test case in `test/rv32i_stall/rv32i_stall.cpp` which contains consecutive instructions that would normally cause pipeline stalls:

```c
int a = 30;     // addi x1, x0, 30
int b = 26;     // addi x2, x0, 26
int c = a + b;  // add x3, x1, x2
int d = c * 2;  // mul x4, x3, x5 (where x5 contains 2)
```

Without hazard detection, the `add` instruction would read incorrect values from x1 and x2. With forwarding, the values are correctly forwarded from the previous instructions' execute stages.

## Performance Impact
- Reduces pipeline stalls significantly compared to naive implementation
- Maintains correct execution order
- Minimal overhead for hazard-free instruction sequences