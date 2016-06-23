## Notes on data preparation:

### Option 1: 
One time execution and build the input for all applications, run data-preparation.sh 

### Option 2: 
We have splitted the data-preparation.sh into different scripts, one for each application subset:

data-preparation-irregular.sh: for generating and transforming inputs for
irregular reductions, including Moldyn and Euler.

data-preparation-graph.sh: for generating and transforming inputs for graph
applications, including PageRank and SSSP.

data-preparation-spmm.sh: for downloading inputs for SpMM, including
float and double executions.

### For other information including how to run the applications, please refer to EXECUTION_INSTRUCTION.pdf.
Contact the author (lcchen008@gmail.com) if you have further questions.
