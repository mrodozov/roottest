#include "dt_RunDrawTest.C"

#include "TClassTable.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH1.h"
#include "TSystem.h"

#ifndef __CINT__
#include "Event.h"
#endif

TH1* RefClone(TH1* orig) {    
  TH1 *cloned = (TH1*)orig->Clone(); 
  TString name = orig->GetName();
  name.Prepend("ref");
  cloned->SetName(name); 
  cloned->Reset();
  return cloned;
};

TH1* RefClone(TDirectory* from, const char* name) {
  TH1 * orig = (TH1*)from->Get(name);
  if (!orig) {
    cerr << "Missing " << name << " from " << from->GetName() << endl;
    return 0;
  }
  return RefClone(orig);
}

void MakeHisto(TTree *tree, TDirectory* To) {
   
   cout << "Generating histograms from TTree::Draw" << endl;
   TDirectory* where = GenerateDrawHist(tree);
   To->cd();

   Event *event = new Event();
   tree->SetBranchAddress("event",&event);

   //We make clones of the generated histograms
   //We set new names and reset the clones.
   //We want to have identical histogram limits
   TH1F *refNtrack = RefClone(where,"hNtrack");
   TH1F *refGetNtrack = RefClone(where,"hGetNtrack");
   TH1F *refNseg   = RefClone(where,"hNseg");  
   TH1F *refTemp   = RefClone(where,"hTemp");  
   TH1F *refHmean  = RefClone(where,"hHmean"); 
   TH1F *refHAxisMax = RefClone(where,"hHAxisMax"); 
   TH1F *refHAxisGetMax = RefClone(where,"hHAxisGetMax"); 
   TH1F *refHGetAxisGetMax  = RefClone(where,"hHGetAxisGetMax");
   TH1F *refHGetAxisMax  = RefClone(where,"hHGetAxisMax");
   TH1F *refGetHGetAxisMax  = RefClone(where,"hGetHGetAxisMax");
   TH1F *refGetRefHGetAxisMax  = RefClone(where,"hGetRefHGetAxisMax");

   TH1F *refPx     = RefClone(where,"hPx"); 
   TH1F *refPy     = RefClone(where,"hPy");
   TH1F *refPz     = RefClone(where,"hPz"); 
   TH1F *refRandom = RefClone(where,"hRandom");
   TH1F *refMass2  = RefClone(where,"hMass2");
   TH1F *refBx     = RefClone(where,"hBx");
   TH1F *refBy     = RefClone(where,"hBy");
   TH1F *refXfirst = RefClone(where,"hXfirst");
   TH1F *refYfirst = RefClone(where,"hYfirst");
   TH1F *refZfirst = RefClone(where,"hZfirst");
   TH1F *refXlast  = RefClone(where,"hXlast");
   TH1F *refYlast  = RefClone(where,"hYlast");
   TH1F *refZlast  = RefClone(where,"hZlast");
   TH1F *refCharge = RefClone(where,"hCharge");
   TH1F *refNpoint = RefClone(where,"hNpoint");
   TH1F *refValid  = RefClone(where,"hValid");

   TH1F *refFullMatrix   = RefClone(where,"hFullMatrix");
   TH1F *refColMatrix    = RefClone(where,"hColMatrix");
   TH1F *refRowMatrix    = RefClone(where,"hRowMatrix");
   TH1F *refCellMatrix   = RefClone(where,"hCellMatrix");
   TH1F *refFullOper     = RefClone(where,"hFullOper");
   TH1F *refCellOper     = RefClone(where,"hCellOper");
   TH1F *refColOper      = RefClone(where,"hColOper");
   TH1F *refRowOper      = RefClone(where,"hRowOper");
   TH1F *refMatchRowOper = RefClone(where,"hMatchRowOper");
   TH1F *refMatchColOper = RefClone(where,"hMatchColOper");
   TH1F *refRowMatOper   = RefClone(where,"hRowMatOper");
   TH1F *refMatchDiffOper= RefClone(where,"hMatchDiffOper");
   TH1F *refFullOper2    = RefClone(where,"hFullOper2");

   TH1F *refClosestDistance  = RefClone(where,"hClosestDistance");
   TH1F *refClosestDistance2 = RefClone(where,"hClosestDistance2");
   TH1F *refClosestDistance9 = RefClone(where,"hClosestDistance9");

   TH1F *refClosestDistanceIndex = RefClone(where, "hClosestDistanceIndex");
   TH2F *refPxInd = (TH2F*)RefClone(where,"hPxInd");

   TH1F *refSqrtNtrack = RefClone(where,"hSqrtNtrack");
   TH1F *refShiftValid = RefClone(where,"hShiftValid");
   TH1F *refAndValid = RefClone(where,"hAndValid");

   TH1F *refString = RefClone(where,"hString");

   TH1F *refPxBx = RefClone(where,"hPxBx");
   TH1F *refPxBxWeight =  RefClone(where,"hPxBxWeight");

   // Loop with user code on all events and fill the ref histograms
   // The code below should produce identical results to the tree->Draw above

   cout << "Recalculating the histograms with custom loop." << endl;

   TClonesArray *tracks = event->GetTracks();
   Int_t nev = (Int_t)tree->GetEntries();
   Int_t i, ntracks, evmod,i0,i1, Nvertex;
   Track *t;
   EventHeader *head;
   Int_t nbin = 0;
   for (Int_t ev=0;ev<nev;ev++) {
      nbin += tree->GetEntry(ev);
      head = event->GetHeader();
      evmod = head->GetEvtNum()%10;
      refNtrack->Fill(event->GetNtrack());
      refGetNtrack->Fill(event->GetNtrack());
      refNseg->Fill(event->GetNseg());
      refTemp->Fill(event->GetTemperature());
      refHmean->Fill(event->GetHistogram()->GetMean());
      refHAxisMax->Fill(event->GetHistogram()->GetXaxis()->GetXmax());
      refHAxisGetMax->Fill(event->GetHistogram()->GetXaxis()->GetXmax());
      refHGetAxisGetMax->Fill(event->GetHistogram()->GetXaxis()->GetXmax());
      refHGetAxisMax->Fill(event->GetHistogram()->GetXaxis()->GetXmax());
      refGetHGetAxisMax->Fill(event->GetHistogram()->GetXaxis()->GetXmax());
      refGetRefHGetAxisMax->Fill(event->GetHistogram()->GetXaxis()->GetXmax());
      refSqrtNtrack->Fill(sqrt(event->GetNtrack()));
      
      if (!strcmp("type1",event->GetType())) 
        refString->Fill(event->GetHeader()->GetEvtNum());
      if (strstr(event->GetType(),"1"))
        refString->Fill(event->GetHeader()->GetEvtNum());

      Nvertex = event->GetNvertex();
      for(i0=0;i0<Nvertex;i0++) {
         refClosestDistance->Fill(event->GetClosestDistance(i0));
      }
      if (Nvertex>2) refClosestDistance2->Fill(event->GetClosestDistance(2));
      if (Nvertex>9) refClosestDistance9->Fill(event->GetClosestDistance(9));
      refClosestDistanceIndex->Fill(event->GetClosestDistance(Nvertex/2));

      for(i0=0;i0<4;i0++) {
         for(i1=0;i1<4;i1++) {
            refFullMatrix->Fill(event->GetMatrix(i0,i1));
         }
         refColMatrix->Fill(event->GetMatrix(i0,0));
         refRowMatrix->Fill(event->GetMatrix(1,i0)); // done here because the matrix is square!
      }
      refCellMatrix->Fill(event->GetMatrix(2,2));

      ntracks = event->GetNtrack();
      if ( 5 < ntracks ) {
         t = (Track*)tracks->UncheckedAt(5);
         for(i0=0;i0<4;i0++) {
            for(i1=0;i1<4;i1++) {
            }
            refColOper->Fill( event->GetMatrix(i0,1) - t->GetVertex(1) );
            refRowOper->Fill( event->GetMatrix(2,i0) - t->GetVertex(2) );
         }
         for(i0=0;i0<3;i0++) {
            refMatchRowOper->Fill( event->GetMatrix(2,i0) - t->GetVertex(i0) );
            refMatchDiffOper->Fill( event->GetMatrix(i0,2) - t->GetVertex(i0) );
         }
         refCellOper->Fill( event->GetMatrix(2,1) - t->GetVertex(1) );
      }
      for (i=0;i<ntracks;i++) {
         t = (Track*)tracks->UncheckedAt(i);
         if (evmod == 0) refPx->Fill(t->GetPx());
         if (evmod == 0) refPy->Fill(t->GetPy());
         if (evmod == 0) refPz->Fill(t->GetPz());
         if (evmod == 1) refRandom->Fill(t->GetRandom());
         if (evmod == 1) refMass2->Fill(t->GetMass2());
         if (evmod == 1) refBx->Fill(t->GetBx());
         if (evmod == 1) refBy->Fill(t->GetBy());
         if (evmod == 2) refXfirst->Fill(t->GetXfirst());
         if (evmod == 2) refYfirst->Fill(t->GetYfirst());
         if (evmod == 2) refZfirst->Fill(t->GetZfirst());
         if (evmod == 3) refXlast->Fill(t->GetXlast());
         if (evmod == 3) refYlast->Fill(t->GetYlast());
         if (evmod == 3) refZlast->Fill(t->GetZlast());
         if (t->GetPx() < 0) {
            refCharge->Fill(t->GetCharge());
            refNpoint->Fill(t->GetNpoint());
            refValid->Fill(t->GetValid());
         }
         Int_t valid = t->GetValid();
         refShiftValid->Fill(valid << 4);
         refShiftValid->Fill( (valid << 4) >> 2 );
         if (event->GetNvertex()>10 && event->GetNseg()<=6000) {
            refAndValid->Fill( t->GetValid() & 0x1 );
         }
         
         Track * t2 = (Track*)tracks->At(t->GetNpoint());
         if (t2 && t2->GetPy()>0) {
            refPxInd->Fill(t2->GetPy(),t->GetPx());
         }
         float Bx,By;
         Bx = t->GetBx();
         By = t->GetBy();
         if ((Bx>.4) || (By<=-.4)) refPxBx->Fill(t->GetPx());
         double weight = Bx*Bx*(Bx>.4) + By*By*(By<=-.4);
         if (weight) refPxBxWeight->Fill(t->GetPx(),weight);

         if (i<4) {
            for(i1=0;i1<3;i1++) { // 3 is the min of the 2nd dim of Matrix and Vertex
               refFullOper ->Fill( event->GetMatrix(i,i1) - t->GetVertex(i1) );
               refFullOper2->Fill( event->GetMatrix(i,i1) - t->GetVertex(i1) );
               refRowMatOper->Fill( event->GetMatrix(i,2) - t->GetVertex(i1) );
            }
            refMatchColOper->Fill( event->GetMatrix(i,2) - t->GetVertex(1) );
         }
      }
   }

   delete event;
   Event::Reset();

}

void dt_MakeRef(const char* from) {
   if (!TClassTable::GetDict("Event")) {
      gSystem->Load("libEvent");
      gHasLibrary = kTRUE;
   }

   gROOT->GetList()->Delete();

   TFile *hfile = new TFile(from);
   TTree *tree = (TTree*)hfile->Get("T");
   
   TFile* f= new TFile("dt_reference.root","recreate");
   MakeHisto(tree,f);
   f->Write();
   delete f;

   delete hfile;

   gROOT->cd();
   cout << "Checking histograms" << endl;
   Compare(gDirectory);

    //   gROOT->GetList()->Delete();

}
