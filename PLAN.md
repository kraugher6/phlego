# Phlego RISC-V Emulator - Development Plan

Questo documento delinea le fasi rimanenti per l'evoluzione dell'emulatore Phlego, integrando architetture avanzate e funzionalità di sistema per l'apprendimento delle architetture dei calcolatori.

## Stato Attuale (Fasi 1-6 Completate)
- [x] **Fase 1**: Interprete sequenziale funzionale (RV32IM).
- [x] **Fase 2**: Pipeline classica a 5 stadi (in-order, naive).
- [x] **Fase 3**: Hazard Detection (RAW) e gestione Stall.
- [x] **Fase 4**: Data Forwarding (EX->EX, MEM->EX).
- [x] **Fase 5**: Branch Prediction (BTB + 2-bit Bimodal Predictor) e Pipeline Flush.
- [x] **Fase 6**: Cache L1 (I-Cache e D-Cache, associativa, LRU, Miss Penalty).

---

## Prossime Fasi

### Fase 7: Superscalar In-Order (2-wide)
**Obiettivo**: Superare il limite di IPC = 1.0 tramite il parallelismo a livello di istruzione.
- **Implementazione**:
    - Aggiornamento Fetch per prelevare 2 istruzioni (8 byte) per ciclo.
    - Logic di **Dual-Issue**: decidere se due istruzioni possono avanzare insieme o se la seconda deve stallare (Structural & Data Hazards tra le due).
    - Raddoppio delle risorse (es. due ALU o una ALU e un'unità Load/Store).
- **Metriche**: Utilizzo degli slot di issue, IPC > 1.

### Fase 8: Out-of-Order Execution - Scoreboard (CDC 6600 style)
**Obiettivo**: Introdurre l'esecuzione fuori ordine in modo graduato.
- **Implementazione**:
    - Introduzione dello **Scoreboard** per tracciare lo stato delle unità funzionali e dei registri.
    - Gestione esplicita di RAW, WAR e WAW hazards tramite Scoreboard.
    - Separazione delle unità funzionali (ALU, MEM, MUL) con diverse latenze.
    - Fase di Issue -> Read Operands -> Execution -> Write Result.

### Fase 9: Out-of-Order Completo - Tomasulo + ROB
**Obiettivo**: Implementare un processore moderno ad alte prestazioni.
- **Implementazione**:
    - **Reservation Stations**: Eliminazione dei WAR/WAW tramite register renaming implicito.
    - **Register Alias Table (RAT)**: Mappatura registri architettonici -> fisici.
    - **Common Data Bus (CDB)**: Broadcast dei risultati alle unità in attesa.
    - **Reorder Buffer (ROB)**: Commit in-order per supportare eccezioni precise e speculazione corretta.
    - Esecuzione speculativa integrata con il branch predictor.

### Fase 10: Estensioni ISA (RV32M + RV32F)
**Obiettivo**: Supporto a benchmark complessi e calcolo scientifico.
- **Implementazione**:
    - **RV32M**: Implementazione di moltiplicatori e divisori multi-ciclo come unità funzionali OoO.
    - **RV32F**: Introduzione del Floating Point Register File (f0-f31) e unità FPU pipelined.

### Fase 11: System Level (CSR, ecall, Privileged)
**Obiettivo**: Permettere l'esecuzione di codice reale e interazione con l'OS.
- **Implementazione**:
    - **CSR (Control and Status Registers)**: Registri di stato, contatori di cicli/istruzioni.
    - **Istruzione `ecall`**: Proxy per system call (es. print char, exit) per caricare ed eseguire programmi C compilati con `riscv-gcc`.
    - Gestione base delle eccezioni e dei trap handler.

### Fase 12: Configurazione e Analisi Comparativa
**Obiettivo**: Rendere l'emulatore uno strumento di analisi professionale.
- **Implementazione**:
    - **JSON Config**: Caricamento di parametri (dimensioni cache, latenze, larghezza pipeline) da file esterni.
    - **Advanced Reporting**: Generazione di report dettagliati (CPI breakdown, heatmap di accesso memoria).
    - **Compare Mode**: Eseguire lo stesso binario su diverse configurazioni e produrre una tabella comparativa delle performance.

---

## Architettura del Codice (Target Finale)
```
riscv-sim/
├── src/
│   ├── core/           # Pipeline, ROB, RAT
│   ├── stages/         # Fetch, Decode, Rename, Issue, Commit
│   ├── hazard/         # Scoreboard, Forwarding Logic
│   ├── prediction/     # BTB, BHT, Branch Logic
│   ├── ooo/            # Reservation Stations, CDB
│   ├── cache/          # L1I, L1D, Cache Controllers
│   ├── metrics/        # Stats Collector
│   └── utils/          # ELF Loader, Logger
```
