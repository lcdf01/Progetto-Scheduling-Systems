// File RA_Helpers.hh
#ifndef RA_HELPERS_HH
#define RA_HELPERS_HH

#include "RA_Data.hh"
#include <easylocal.hh>

using namespace EasyLocal::Core;

class RA_Change
{
  friend bool operator==(const RA_Change& m1, const RA_Change& m2);
  friend bool operator!=(const RA_Change& m1, const RA_Change& m2);
  friend bool operator<(const RA_Change& m1, const RA_Change& m2);
  friend ostream& operator<<(ostream& os, const RA_Change& c);
  friend istream& operator>>(istream& is, RA_Change& c);
 public:
  string game, old_ref, new_ref;
  RA_Change();
};

/***************************************************************************
 * State Manager 
 ***************************************************************************/

class RA_SolutionManager : public SolutionManager<RA_Input,RA_Output> 
{
public:
  RA_SolutionManager(const RA_Input &);
  void RandomState(RA_Output& out) override;   
  void GreedyState(RA_Output& out) override;   
  void DumpState(const ra_Output& out, ostream& os) const override { out.Dump(os); }   
  bool CheckConsistency(const RA_Output& st) const override;
protected:
}; 

class RA_Supply : public CostComponent<RA_Input,RA_Output> 
{
public:
  RA_Supply(const RA_Input & in, int w, bool hard) :    CostComponent<RA_Input,RA_Output>(in,w,hard,"RA_Supply") 
  {}
  int ComputeCost(const RA_Output& st) const override;
  void PrintViolations(const RA_Output& st, ostream& os = cout) const override;
};

class  RA_Opening: public CostComponent<RA_Input,RA_Output> 
{
public:
  RA_Opening(const RA_Input & in, int w, bool hard) : CostComponent<RA_Input,RA_Output>(in,w,hard,"RA_Opening") 
  {}
  int ComputeCost(const RA_Output& st) const override;
  void PrintViolations(const RA_Output& st, ostream& os = cout) const override;
};

class  RA_Capacity: public CostComponent<RA_Input,RA_Output> 
{
public:
  RA_Capacity(const RA_Input & in, int w, bool hard) : CostComponent<RA_Input,RA_Output>(in,w,hard,"RA_Capacity") 
  {}
  int ComputeCost(const RA_Output& st) const override;
  void PrintViolations(const RA_Output& st, ostream& os = cout) const override;
};

/***************************************************************************
 * RA_Change Neighborhood Explorer:
 ***************************************************************************/

class RA_ChangeDeltaSupply
  : public DeltaCostComponent<RA_Input,RA_Output,RA_Change>
{
public:
  RA_ChangeDeltaSupply(const RA_Input & in, RA_Supply& cc) 
    : DeltaCostComponent<RA_Input,RA_Output,RA_Change>(in,cc,"RA_ChangeDeltaSupply") 
  {}
  int ComputeDeltaCost(const RA_Output& st, const RA_Change& mv) const override;
};

class RA_ChangeDeltaOpening
  : public DeltaCostComponent<RA_Input,RA_Output,RA_Change>
{
public:
  RA_ChangeDeltaOpening(const RA_Input & in, RA_Opening& cc) 
    : DeltaCostComponent<RA_Input,RA_Output,RA_Change>(in,cc,"RA_ChangeDeltaOpening") 
  {}
  int ComputeDeltaCost(const RA_Output& st, const RA_Change& mv) const override;
};

class RA_ChangeDeltaCapacity
  : public DeltaCostComponent<RA_Input,RA_Output,RA_Change>
{
public:
  RA_ChangeDeltaCapacity(const RA_Input & in, RA_Capacity& cc) 
    : DeltaCostComponent<RA_Input,RA_Output,RA_Change>(in,cc,"RA_ChangeDeltaCapacity") 
  {}
  int ComputeDeltaCost(const RA_Output& st, const RA_Change& mv) const override;
};

class RA_ChangeNeighborhoodExplorer
  : public NeighborhoodExplorer<RA_Input,RA_Output,RA_Change> 
{
public:
  RA_ChangeNeighborhoodExplorer(const RA_Input & pin, SolutionManager<RA_Input,RA_Output>& psm)  
    : NeighborhoodExplorer<RA_Input,RA_Output,RA_Change>(pin, psm, "RA_ChangeNeighborhoodExplorer") {} 
  void RandomMove(const RA_Output&, RA_Change&) const override;          
  bool FeasibleMove(const RA_Output&, const RA_Change&) const override;  
  void MakeMove(RA_Output&, const RA_Change&) const override;             
  void FirstMove(const RA_Output&, RA_Change&) const override;  
  bool NextMove(const RA_Output&, RA_Change&) const override;   
protected:
  bool AnyNextMove(const RA_Output&, RA_Change&) const;   
};

#endif
