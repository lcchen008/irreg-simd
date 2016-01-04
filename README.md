## Notes on data preparation:

### Option 1: 
    One time execution and build the input for all applications, run _data-preparation.sh_ 

### Option 2: 
    We have splitted the data-preparation.sh into different scripts, one for each application subset.

    + _data-preparation-irregular.sh_ is for generating and transforming inputs for irregular
      reductions, including Moldyn and Euler.

    + _data-preparation-graph.sh_ is for generating and transforming inputs for graph applications,
      including PageRank and SSSP.

    + _data-preparation-spmm.sh_ is for downloading inputs for SpMM, including float and double
      executions.

   For other information including how to run the applications, please refer to EXECUTION_INSTRUCTION.pdf.
   Contact the author (lcchen008@gmail.com) if you have further questions.
