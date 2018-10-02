void ParticleSelectionSorted_NoDebug() 
{
	const Int_t maxClusters = 1000;        //Maximum number of clusters per event (to create the arrays), there are usually around 300 at 80 GeV.
	const Int_t nRPCs = 48;		       //Number of RPCs (layers)
	const Int_t maxRPCswoS = 3;            //Maximum number of layers without signal to consider the signals to be neighbours
	const Int_t mincRPCswS = 5;	       //Minimum number of neighbours to consider the signal valid

	const Long64_t cnEntries = 999999;     //Maximum number of entries per event

	const Double_t xUpperLimit = 1500;     //This is the upper limit in the X axis for the histogram, for high energies it has to be increased

	TFile *dataFile = gFile;                             //Copy the data file direction to restore the scope when the macro ends
	TTree* hcal = (TTree*)dataFile->Get("DHCALC");       //This Tree contains the data

	Long64_t nEntries = hcal->GetEntries();

	//Variables which will receive the branches of each entry
	Int_t nHit;
	Int_t nClusters;
	Float_t clusterZPos[maxClusters] = {};

	//Asigning the previous variables to its corresponding branch in the Tree
	hcal->SetBranchAddress("Nhit", &nHit);
	hcal->SetBranchAddress("Clusters", &nClusters);
	hcal->SetBranchAddress("ZposCluster", &clusterZPos);

	gStyle->SetOptStat(0);    //Removing the statistics box from the histograms

	//Creation of the different histograms and the legend
	TH1I *nHist = new TH1I("nHits", "nHits", 150, 0., xUpperLimit); //This histogram holds the Nhit distribution of all events
	nHist->SetFillColor(kOrange);
	nHist->SetLineColor(kOrange);
	nHist->GetXaxis()->SetTitle("Nhit");
	nHist->GetXaxis()->SetNdivisions(5,5,0);
	nHist->GetYaxis()->SetTitle("Entries");

	TH1I *nHitProb_Cosm = new TH1I("Cosmic&Problems", "Cosmic&Problems", 150, 0., xUpperLimit); //Here all the noise, problematic events and some cosmic rays are stored
	nHitProb_Cosm->SetFillColor(kBlack);
	nHitProb_Cosm->SetFillStyle(3005);
	nHitProb_Cosm->SetLineColor(kBlack);
	nHitProb_Cosm->SetLineWidth(3);

	TH1I *nHitBeam = new TH1I("BeamParticles", "BeamParticles", 150, 0., xUpperLimit);  //The beam particles plus the remaining cosmic rays are held in this histogram
	nHitBeam->SetLineColor(kBlue);
	nHitBeam->SetLineWidth(3);

	TLegend* legend = new TLegend(0.52, 0.62, 0.87, 0.87); //A legend to distinguish each histogram in the plots
	legend->AddEntry(nHist, "Nhit Total");
	legend->AddEntry(nHitProb_Cosm, "Nhit Prob + Cosmic");
	legend->AddEntry(nHitBeam, "Nhit Beam Particles");

	TString outputFileName = "/afs/ciemat.es/user/h/hectorgc/Documentos/Data/Processed/Processed_";    //IMPORTANT: set the path and prefix to the output file name
	outputFileName += gDirectory->GetName();             

	//Output file whith the previous path where the cloneTree will be written
	TFile* outputFile = new TFile(outputFileName, "recreate");
	TTree *cloneTree = hcal->CloneTree(0);

	Int_t nNotValidEvents = 0;      //This is to later check the percentage of events removed from the data

	cout << "BEGIN" << endl;

	for(Long64_t i = 0; i < nEntries; i++) {
	  
	  hcal->GetEntry(i);

	  nHist->Fill(nHit);
	  
	  /* -------------------------------------------------------------
	     To understand the analysis of the data one must know that 
	     we assume the clusters to be ordered in the RPC direction.
	     This means that clusterZPos[i] <= clusterZPos[j] if j > i.
	     -------------------------------------------------------------*/

	  /* -------------------------------------------------------------
	     To reconstruct a physical process and distinguish it from the background
	     we select those events that have more than 7 hits in total
	     --------------------------------------------------------------*/
	  
	  if(nHit < 7) 
	    { 
	      DisplayAdvance(i,nEntries);
	      nHitProb_Cosm->Fill(nHit);
	      nNotValidEvents++;
	      continue;
	    }

	  /* -------------------------------------------------------------
	     Due to the RPC's high detection efficency, the probability to
	     find an event produced by a particle from the beam which 
	     doesn't leave signal in the first to layers is too low (~ 0.36 %).
	     We assume that a beam particle leaves signal in the first two layers.
	     -------------------------------------------------------------*/

	  if( clusterZPos[0] > 2) 
	    {
	      DisplayAdvance(i,nEntries);
	      nHitProb_Cosm->Fill(nHit);
	      nNotValidEvents++;
	      continue;
	    }

	  /* ------------------------------------------------------------
	     Selection A:
	     In the first few layers, before the cascade has happened, the
	     energy deposited is very low. From the accumulated charge in the
	     pads, there could be a local reduction of the electric field and
	     a loss in efficency, pronounced at the detector's beginning.
	     To elminate this situation we require 4 RPCs with signal between
	     the first 10 layers and 3 RPCs with signal among the first 6. 
	     ------------------------------------------------------------*/

	  Int_t nRPCsitB_10 = 0;
	  Int_t nRPCsitB_6 = 0;

	  /* ------------------------------------------------------------
	     Selection B:
	     To be able to reconstruct the trace of the particle we require
	     at least 5 close RPCs with signal. It is said that two RPC are 
	     close if there are less than 3 RPCs without signal in between
	     them. We call a "set of close RPCs" (setocRPCs) to a packet of more than 5
	     close RPCs with signal and there could be more than one in a 
	     single event.
	     ------------------------------------------------------------*/

	  Float_t setocRPCs[6][nRPCs] = {};      //The prototype has 48 layers then the maximum amount of setocRPCs is 6
	  Int_t nsetocRPCs = 0;

	  //Here a copy of clusterZPos is made to work with it without changing the values in the tree
	  Float_t clusterZPosCopy[maxClusters] = {};

	  for(int p = 0; p < nClusters; p++)
	    {
	      clusterZPosCopy[p] = clusterZPos[p];
	    }	

	  //(B) The following section just calculates the setocRPCs
	  Float_t cRPCs[nRPCs] = {};

	  for(int m = 0; m < nClusters; m++)
	    {
	      if( clusterZPosCopy[m] == -999.) { continue; }		
	      
	      //(A) Here we check if there is signal at the beginning of the prototype
	      if( clusterZPosCopy[m] <= 10)
		{
		  if( clusterZPosCopy[m] <= 6) { nRPCsitB_6++; }
		  nRPCsitB_10++;
		}

	      cRPCs[0] = clusterZPosCopy[m];
	      clusterZPosCopy[m] = -999.;
	      Int_t cRPCsFound = 1;
		
	      for(int l = m + 1; l < nClusters; l++)
		{
		  //(A) The same as before but because we eliminate a value one worked with there is no prblem with adding here twice
		  if( clusterZPosCopy[l] <= 10)
		    {
		      if( clusterZPosCopy[l] <= 6) { nRPCsitB_6++; }
		      nRPCsitB_10++;
		    }
		  
		  if(clusterZPosCopy[l] == cRPCs[cRPCsFound - 1]) {clusterZPosCopy[l] = -999.; continue;}
		  if(TMath::Abs(clusterZPosCopy[l] - cRPCs[cRPCsFound - 1]) < maxRPCswoS)      //The Abs is redundant because we already know the data is in order
		    {
		      cRPCs[cRPCsFound] = clusterZPosCopy[l];
		      clusterZPosCopy[l] = -999.;
		      cRPCsFound++;
		    }
		  else { break; }

		}

	      //If the set found has more than 5 RPCs with add it to the list
	      if(cRPCsFound >= mincRPCswS) 
		{ 	
		  for(int s = 0; s < cRPCsFound; s++)
		    {
		      setocRPCs[nsetocRPCs][s] = cRPCs[s]; 
		    }
		  nsetocRPCs++;
		}
	      
	    }

	  //(A) If the conditions aren't fullfiled we remove this event
	  if(nRPCsitB_10 < 4 || nRPCsitB_6 < 3)
	    {
	      DisplayAdvance(i, nEntries);
	      nHitProb_Cosm->Fill(nHit);
	      nNotValidEvents++;
	      continue;
	    }

	  /* --------------------------------------------------------
	     (B.2) To eliminate observed problems with the electronics or the DAQ
	     were the signal is lost in consecutve planes, even inside the cascades,
	     we request that the distance between two setocRPCs is no larger than 5
	     layers and also that there are no more than 3 sets.
	     --------------------------------------------------------*/
	  Bool_t setsepbig = false;
	
	  for( int g = 1; g < nsetocRPCs; g++)
	    {
	      Float_t lastRPC = 0.;
	      for(int h = 0; h < nRPCs; h++) { if(setocRPCs[g-1][h] != 0) { lastRPC = setocRPCs[g-1][h]; } }
	      if(TMath::Abs( lastRPC - setocRPCs[g][0]) > 5) { setsepbig = true; }
	    }

	  if(setsepbig || nsetocRPCs > 3 || nsetocRPCs == 0) 
	    {
	      DisplayAdvance(i,nEntries);
	      nHitProb_Cosm->Fill(nHit);
	      nNotValidEvents++;
	      continue;
	    }
		
	  //Finally the remaining events are considered beam particles and are saved to the new tree
	  nHitBeam->Fill(nHit);
	  cloneTree->Fill();
		
	  DisplayAdvance(i,nEntries);
		
	}

	cout << "END" << endl;

	cout << nNotValidEvents/(Double_t)nEntries*100 << "% of the events removed." << endl;

	//Write the new tree to the disk
	cloneTree->Write();

	//Creating a canvas and drawing the histograms and the legend into it
	TCanvas *C1 = new TCanvas();
	c1->SetLogy();
	
	nHist->DrawCopy();
	nHitProb_Cosm->DrawCopy("same");
	nHitBeam->DrawCopy("same");
	legend->DrawClone();

	//Clearing the direction to read the branches from the tree because the variables will be eliminated when the function ends
	hcal->ResetBranchAddresses();

	dataFile->cd();      //Before ending the macro restore the working directory to to the data
	
	//Freeing the memory taken by the pointers before ending the function (The canvas is an exception because it is closed manually)
	delete hcal;
	delete cloneTree;
	delete outputFile;
	delete nHist;
	delete nHitProb_Cosm;
	delete nHitBeam;
	delete legend;
}


void DisplayAdvance(Int_t current, Int_t total)
{
  Int_t dispCuantity = 10000;     //Every time this many events are proccessed display how much of the total is complete
  
  if(current == 0) { return; }
  if(current%dispCuantity == 0) { cout << current << " of " << total << endl;}
}


