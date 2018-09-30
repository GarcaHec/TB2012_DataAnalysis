# SDHCAL TB2012_DataAnalysis

*TreeBranchesDescription* 

This file has all the information about the format of the variables inside the Trees with data, which is in 
the .root file. The data file is just a small sample to run some tests if needed.

-------------------------------------------------------------------------------------------------------------

*BeamParticleSelection_DataSorted.C*

This macro takes the Trees whitin the data, which have the clusters already constructed, and rejects those 
that are noise, problems with the electronics and the cosmic rays that don't pass throught the beginning of 
the prototype. Then it outputs another file with the same structure as the data but with only the events 
selected as beam particles.

-------------------------------------------------------------------------------------------------------------

To run a macro just open the .root file with root and then execute the macro using the command:

.x Path to macro
