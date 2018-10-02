void MuonSelection_NoDebug() 
{
	const Int_t maxClusters = 1000;  	//Maximum number of clusters in the arrays per event
	const Int_t maxHits = 10000;		//Maximum number of hits      ''
	const Int_t nRPCs = 48;			//Total number of RPCs in the detector (layers)
	const Int_t nPlanesforTrace = 10;	// ------- WHAT IS THIS? 

	const Double_t xUpperLimit = 1500.;     //This is the upper limit in the X axis for the histogram, for high energies it has to be increased
	
	TFile *dataFile = gFile;                //Copy the data file direction to restore the scope when the macro ends
	if(!dataFile) { cout << "Bad Data file" << endl; break;}


	TTree* hcal = (TTree*)gFile->Get("DHCALC");   //Copying the tree from the file

	Long64_t nEntries = hcal->GetEntries(); //Getting the number of entries in the tree

	//Variables to read into the data from the branches

	Int_t nHit = 0;
	Int_t nClusters = 0;
	Int_t nHitsCluster[maxClusters] = {};
	Float_t clusterZPos[maxClusters] = {};
	Int_t hitXPos[maxHits] = {};
	Int_t hitYPos[maxHits] = {};
	Int_t hitRPCPos[maxHits] = {};

	//Asigning the previous variables to its corresponding branches in the Tree
	hcal->SetBranchAddress("Nhit", &nHit);
	hcal->SetBranchAddress("Clusters", &nClusters);
	hcal->SetBranchAddress("ZposCluster", &clusterZPos);
	hcal->SetBranchAddress("NHitsCluster", &nHitsCluster);
	hcal->SetBranchAddress("DHCALEvent.I", &hitXPos);
	hcal->SetBranchAddress("DHCALEvent.J", &hitYPos);
	hcal->SetBranchAddress("DHCALEvent.K", &hitRPCPos);

	gStyle->SetOptStat(0);   //Remove the statistics box

	//Histogram which includes all the events
	TH1I *nHitBeamParticles = new TH1I("BeamParticles", "BeamParticlesHits", 120, 0., xUpperLimit);
	nHitBeamParticles->SetFillColor(kOrange);
	nHitBeamParticles->SetLineColor(kOrange);
	nHitBeamParticles->GetXaxis()->SetTitle("Nhit");
	nHitBeamParticles->GetXaxis()->SetNdivisions(5,5,0);
	nHitBeamParticles->GetYaxis()->SetTitle("Entries");

	//All the events idfentified as cosmic rays are put into this histogram
	TH1I *nHitCosmic = new TH1I("CosmicRays", "CosmicRays", 120, 0., xUpperLimit);
	nHitCosmic->SetFillColor(kBlack);
	nHitCosmic->SetFillStyle(3005);
	nHitCosmic->SetLineColor(kBlack);
	nHitCosmic->SetLineWidth(3);

	//Here are all the events selected as muons
	TH1I *nHitMuons = new TH1I("Muons", "Muons", 120, 0., xUpperLimit);
	nHitMuons->SetLineColor(kBlue);
	nHitMuons->SetLineWidth(3);

	//The legend clarifies the meaning of each part in the plot
	TLegend* legend_1 = new TLegend(0.52, 0.62, 0.87, 0.87);
	legend_1->AddEntry(nHitBeamParticles, "Nhit Beam");
	legend_1->AddEntry(nHitCosmic, "Nhit Cosmic Rays");
	legend_1->AddEntry(nHitMuons, "Nhit Muons");

	/* ---------------------------------------------------------
	   After the first set of selection rules we are left with 
	   events that produced a cascade in the detector and some
	   remaining cosmic rays.
	   ---------------------------------------------------------*/

	//In this Histogram contains the events identified as cascades
	TH1I *nHitCascades = new TH1I("Cascades", "Cascades", 120, 0., 2500.);
	nHitCascades->SetFillColor(kOrange);
	nHitCascades->SetLineColor(kOrange);
	nHitCascades->GetXaxis()->SetTitle("Nhit");
	nHitCascades->GetXaxis()->SetNdivisions(5,5,0);
	nHitCascades->GetYaxis()->SetTitle("Entries");

	/*Analizing the angular distribution from the trayectories of the particles we
	 identify most of the remaining cosmic rays and are sotred in the following histogram.*/
	TH1I *nHitCosmicAngular = new TH1I("AngularCosmicRays", "AngularCosmicRays", 120, 0., 2500.);
	nHitCosmicAngular->SetFillColor(kBlack);
	nHitCosmicAngular->SetFillStyle(3005);
	nHitCosmicAngular->SetLineColor(kBlack);
	nHitCosmicAngular->SetLineWidth(3);

	//Due to its high penetrating power we can select the muons which are stored here
	TH1I *nHitMuonsCascades = new TH1I("MuonCascades", "MuonCascades", 120, 0., 2500.);
	nHitMuonsCascades->SetLineColor(kBlue);
	nHitMuonsCascades->SetLineWidth(3);

	//Another legend to clarify the plots
	TLegend* legend_2 = new TLegend(0.52, 0.62, 0.87, 0.87);
	legend_2->AddEntry(nHitCascades, "Nhit All Cascades");
	legend_2->AddEntry(nHitCosmicAngular, "Nhit Cosmic Angular");
	legend_2->AddEntry(nHitMuonsCascades, "Nhit Muons Cascades");

	//This is the steep distribution for the x_z trace reconstruction
	TH1I *steep_x = new TH1I("steep_x", "steep_x", 100, -2., 2.);
	steep_x->SetFillColor(kBlue);
	steep_x->SetFillStyle(3005);
	steep_x->SetLineColor(kBlue);
	steep_x->SetLineWidth(2);
	steep_x->GetXaxis()->SetTitle("m_x");
	steep_x->GetYaxis()->SetTitle("Entries");

	//Same as the previous one but for the y_z
	TH1I *steep_y = new TH1I("steep_y", "steep_y", 100, -2., 2.);
	steep_y->GetXaxis()->SetTitle("m_y");
	steep_y->GetYaxis()->SetTitle("Entries");

	//IMPORTANT: set the path and prefix to the output file name
	//We will write to this files all the muons and cosmic rays after the whole set of selection rules
	TString outputFileName_Muons = "/afs/ciemat.es/user/h/hectorgc/Documentos/Data/Muons/Muons_";
	TString outputFileName_Cosmic = "/afs/ciemat.es/user/h/hectorgc/Documentos/Data/CosmicRays/Cosmic_";
	outputFileName_Muons += gDirectory->GetName();
	outputFileName_Cosmic += gDirectory->GetName();

	//Creation of the output files.
	/* ----------------------------------------------------------------
	   IMPORTANT: When a new TFile is created Root changes its working
	   directory to the file so we will be using "cd" to switch between
	   files when needed.
	   ----------------------------------------------------------------*/
	TFile* outputFile_Cosmic = new TFile(outputFileName_Cosmic, "recreate");
	TTree *cloneTree_Cosmic = hcal->CloneTree(0);
	TFile* outputFile_Muons = new TFile(outputFileName_Muons, "recreate");
	TTree *cloneTree_Muons = hcal->CloneTree(0);

	Int_t nMuons = 0;    //Number of events selected as muons
	Int_t nCRays = 0;    //Number of events selected as cosmic rays

	cout << "BEGIN" << endl;

	for(Long64_t i = 7; i < nEntries; i++) {

		hcal->GetEntry(i);

		nHitBeamParticles->Fill(nHit);

		Float_t density = 0.;
		Int_t nPlanes = 0;
		Int_t currentRPC = 0;

		Int_t maxHits_1 = 0;
		Int_t maxHits_2 = 0;

		Int_t region_A = 0;
		Int_t region_B = 0;
		Int_t region_C = 0;
		Int_t region_D = 0;
		Bool_t penetrability_condition = false;		

		for(int m = 0; m < nClusters; m++)
		{				
			if(clusterZPos[m] != currentRPC) { nPlanes++; currentRPC = clusterZPos[m]; }

			if(nHitsCluster[m] > maxHits_2)
			{	
				if(nHitsCluster[m] > maxHits_1) { maxHits_2 = maxHits_1; maxHits_1 = nHitsCluster[m]; }
				else { maxHits_2 = nHitsCluster[m]; }
			}

			if(clusterZPos[m] <= 10) { region_A++; }
			else if(clusterZPos[m] > 10 && clusterZPos[m] <= 20) { region_B++; }
			else if(clusterZPos[m] > 20 && clusterZPos[m] <= 35) { region_C++; }
			else { region_D++; }
		}

		density = (float)nHit/nPlanes;
		if(region_A >= 7 && region_B >= 7 && region_C >= 9 && region_D >= 8) { penetrability_condition = true; }

		if(density < 2.5 || maxHits_2 < 5) 
		{
			if(penetrability_condition) 
			  { 
			    nHitMuons->Fill(nHit);
			    outputFile_Muons->cd();
			    cloneTree_Muons->Fill();  
			    nMuons++;
			  }
			else 
			  {
			    nHitCosmic->Fill(nHit); 
			    outputFile_Cosmic->cd(); 
			    cloneTree_Cosmic->Fill();
			    nCRays++;
			  }
			DisplayAdvance(i, nEntries);
			continue;
		}
		else if( density < 5 && penetrability_condition) { 
			nHitCascades->Fill(nHit);
			nHitMuonsCascades->Fill(nHit);
			outputFile_Muons->cd();
			cloneTree_Muons->Fill();
			DisplayAdvance(i, nEntries);
			nMuons++;
			continue;
		}

		nHitCascades->Fill(nHit);

		Double_t x_mean[nPlanesforTrace]   = {};
		Double_t y_mean[nPlanesforTrace]   = {};
		Double_t z_pos[nPlanesforTrace]    = {};
		Double_t nHitsRPC[nPlanesforTrace] = {};
		Bool_t there_is_Data[nPlanesforTrace] = {};
		Int_t n = 0;

		TGraph* x_z = new TGraph();
		TGraph* y_z = new TGraph();

		for(int l = 0; l < nHit; l++)
		{
			if(hitRPCPos[l] > nPlanesforTrace) { continue; }
			there_is_Data[hitRPCPos[l] - 1] = true; 
			x_mean[hitRPCPos[l] - 1] += hitXPos[l];
			y_mean[hitRPCPos[l] - 1] += hitYPos[l];
			z_pos[hitRPCPos[l] - 1]   = hitRPCPos[l];
			nHitsRPC[hitRPCPos[l] - 1] += 1.;
		}

		for(int k = 0; k < nPlanesforTrace; k++)
		{
			if(!there_is_Data[k]) { continue; }
			x_z->Set(n);
			x_z->SetPoint(n, z_pos[k], x_mean[k]/nHitsRPC[k]);
			y_z->Set(n);
			y_z->SetPoint(n, z_pos[k], y_mean[k]/nHitsRPC[k]);
			n++;
		}

		Double_t x_param[2] = {};
		Double_t y_param[2] = {};

		x_z->LeastSquareFit(2, x_param);
		y_z->LeastSquareFit(2, y_param);

		Double_t a_xz = x_param[1];
		Double_t b_xz = x_param[0];
		Double_t a_yz = y_param[1];
		Double_t b_yz = y_param[0];

		steep_x->Fill(a_xz);
		steep_y->Fill(a_yz);

		if((TMath::Abs(a_xz) > 0.3 && TMath::Abs(a_yz)> 0.3) || TMath::Abs(a_xz) > 0.5 || TMath::Abs(a_yz)>0.5)	
		{	
		  if(density < 5) 
		    { 
		      
		      Float_t aa[2] = {a_xz, a_yz};
		      Float_t bb[2] = {b_xz, b_yz};
		      outputFile_Cosmic->cd(); 
		      cloneTree_Cosmic->Fill();
		      nHitCosmicAngular->Fill(nHit);
		      nCRays++;
		    }

		  DisplayAdvance(i, nEntries);
		}

		DisplayAdvance(i, nEntries);
	}

	cout << "DONE" << endl;

	cout << nMuons/(Double_t)nEntries*100 << "% of muon in the beam." << endl;
	cout << nCRays/(Double_t)nEntries*100 << "% of cosmic rays in the beam." << endl;
	
	//Writing the trees into the disk
	outputFile_Muons->cd();
	cloneTree_Muons->Write();

	outputFile_Cosmic->cd();
	cloneTree_Cosmic->Write();

	//First canvas for the selection of cascades
	TCanvas *canvas_1 = new TCanvas("First selection");
	canvas_1->SetLogy();

	nHitBeamParticles->DrawCopy();
	nHitCosmic->DrawCopy("same");
	nHitMuons->DrawCopy("same");
	legend_1->DrawClone();

	//Second canvas to draw the separation of the remaining muons and cosmic rays
	TCanvas *canvas_2 = new TCanvas("Cascades");
	canvas_2->SetLogy();
	
	nHitCascades->DrawCopy();
	nHitCosmicAngular->DrawCopy("same");
	nHitMuonsCascades->DrawCopy("same");
	legend_2->DrawClone();

	//One last canvas for the steep distributions
	TCanvas *canvas_3 = new TCanvas("Steep");
	canvas_3->Divide(2,1);
	canvas_3->cd(1)->SetLogy();
	canvas_3->cd(2)->SetLogy();

	canvas_3->cd(1);
	steep_x->DrawCopy();
	canvas_3->cd(2);
	steep_y->DrawCopy();
	canvas_3->Update();
	

	//Clearing the direction to read the branches from the tree because the variables will be eliminated when the function ends
	hcal->ResetBranchAddresses();

	dataFile->cd();  //Returning to the starting directory

	//Freeing the memory taken by the pointers before ending the function (The canvas is an exception because it is closed manually)
	delete hcal;
	delete cloneTree_Muons;
	delete cloneTree_Cosmic;
	delete outputFile_Muons;
	delete outputFile_Cosmic;
	delete nHitBeamParticles;
	delete nHitCosmic;
	delete nHitMuons;
	delete nHitCascades;
	delete nHitCosmicAngular;
	delete nHitMuonsCascades;
	delete steep_x;
	delete steep_y;

}


void DisplayAdvance(Int_t current, Int_t total)
{
  Int_t dispCuantity = 10000;     //Every time this many events are proccessed display how much of the total is complete
  
  if(current == 0) { return; }
  if(current%dispCuantity == 0) { cout << current << " of " << total << endl;}
}




