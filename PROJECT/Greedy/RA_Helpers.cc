// File WL_Helpers.cc
#include "WL_Helpers.hh"

WL_SolutionManager::WL_SolutionManager(const WL_Input & pin) 
  : SolutionManager<WL_Input,WL_Output>(pin, "WLSolutionManager")  {} 

void WL_SolutionManager::RandomState(WL_Output& out) 
{
  unsigned s, w;
  out.Reset();
  for (s = 0; s < in.Stores(); s++)
    {
      w = Random::Uniform<int>(0, in.Warehouses() - 1);
      out.Assign(s,w);
    }
} 

void WL_SolutionManager::GreedyState(WL_Output& out) 
{
  bool found_first;
  unsigned i, s, w, best_s, best_i, best_w;
  double cost, best_cost, amortized_fixed_cost;
  vector<unsigned> unserved_stores(in.Stores());

  out.Reset();
  iota(unserved_stores.begin(), unserved_stores.end(),0);

  while(unserved_stores.size() > 0)
    {
      found_first = false;
      for (i = 0; i < unserved_stores.size(); i++)
        {
          s = unserved_stores[i];
          for (w = 0; w < in.Warehouses(); w++)
            if (out.ResidualCapacity(w) > in.AmountOfGoods(s))
              {
                if (out.Load(w) == 0) 
                  amortized_fixed_cost = in.FixedCost(w) * in.AmountOfGoods(s)/static_cast<double>(in.Capacity(w));
                else
                  amortized_fixed_cost = 0.0;
                if (!found_first)
                  {
                    found_first = true;
                    best_w = w;
                    best_s = s;
                    best_i = i;
                    best_cost = in.SupplyCost(s,w) + amortized_fixed_cost;
                  }
                else
                  {
                    cost = in.SupplyCost(s,w) + amortized_fixed_cost;
                    if (cost < best_cost) 
                      {
                        best_w = w;
                        best_s = s;
                        best_i = i;
                        best_cost = cost;
                      }
                  }
              }
        }
      if (!found_first)
        break; // no feasible assignment found
      out.Assign(best_s,best_w);
      unserved_stores.erase(unserved_stores.begin() + best_i);
    }
  if (unserved_stores.size() > 0) // if there are unassigned stores, assign all of them to warehouse 0
    for (i = 0; i < unserved_stores.size(); i++)
      out.Assign(unserved_stores[i], 0);
}

bool WL_SolutionManager::CheckConsistency(const WL_Output& st) const
{ // not necessary, as state modifications are managed by the single method Assign
  return true;
}

int WL_Supply::ComputeCost(const WL_Output& st) const
{
  unsigned s, cost = 0;
  for (s = 0; s < in.Stores(); s++)
    cost += in.AmountOfGoods(s) * in.SupplyCost(s,st.Assignment(s));
  return cost;
}

void WL_Supply::PrintViolations(const WL_Output& st, ostream& os) const
{
  unsigned s;
  for (s = 0; s < in.Stores(); s++)
    os << "The cost of supplying " << in.AmountOfGoods(s) << " units from " << st.Assignment(s) 
      << " to " << s << " is " << in.AmountOfGoods(s)*in.SupplyCost(s,st.Assignment(s)) 
      << " (cost per unit " << in.SupplyCost(s,st.Assignment(s)) << ")" << endl;	
}

int WL_Opening::ComputeCost(const WL_Output& st) const
{ 
  unsigned w, cost = 0;
  for (w = 0; w < in.Warehouses(); w++)
    if (st.Load(w) > 0)
      cost += in.FixedCost(w);
  return cost;
}

void WL_Opening::PrintViolations(const WL_Output& st, ostream& os) const
{
  unsigned w;
  for (w = 0; w < in.Warehouses(); w++)
    if (st.Load(w) > 0)
      os << "The cost of opening warehouse " << w << " is " << in.FixedCost(w) << endl;
}

int WL_Capacity::ComputeCost(const WL_Output& st) const
{ 
  unsigned w, cost = 0;
  for (w = 0; w < in.Warehouses(); w++)
    if (st.Load(w) > in.Capacity(w))
      cost += st.Load(w) - in.Capacity(w);
  return cost;
}

void WL_Capacity::PrintViolations(const WL_Output& st, ostream& os) const
{
  unsigned w;
  for (w = 0; w < in.Warehouses(); w++)
    if (st.Load(w) > in.Capacity(w))
      os << "Warehouse " << w << " is overloaded by " << st.Load(w) - in.Capacity(w) << endl;
}

/*****************************************************************************
  * WL_Change Neighborhood Methods
  *****************************************************************************/
WL_Change::WL_Change()
{
  store = -1;
  old_w = -1;
  new_w = -1;
}

bool operator==(const WL_Change& mv1, const WL_Change& mv2)
{
  return mv1.store == mv2.store && mv1.new_w == mv2.new_w;
}

bool operator!=(const WL_Change& mv1, const WL_Change& mv2)
{
  return mv1.store != mv2.store || mv1.new_w != mv2.new_w;
}

bool operator<(const WL_Change& mv1, const WL_Change& mv2)
{
  return (mv1.store < mv2.store)
  || (mv1.store == mv2.store && mv1.new_w < mv2.new_w);
}

istream& operator>>(istream& is, WL_Change& mv)
{
  char ch;
  is >> mv.store >> ch >> mv.old_w >> ch >> ch >> mv.new_w;
  return is;
}

ostream& operator<<(ostream& os, const WL_Change& mv)
{
  os << mv.store << ':' << mv.old_w << "->" << mv.new_w;
  return os;
}

void WL_ChangeNeighborhoodExplorer::RandomMove(const WL_Output& st, WL_Change& mv) const
{ 
  mv.store = Random::Uniform<unsigned>(0, in.Stores() - 1);
  mv.old_w = st.Assignment(mv.store);
  
  do{
    mv.new_w = Random::Uniform<unsigned>(0, in.Warehouses() - 1);
  }while (mv.new_w == mv.old_w); // assicura che le due warehouse siano diverse
} 

bool WL_ChangeNeighborhoodExplorer::FeasibleMove(const WL_Output& st, const WL_Change& mv) const
{
  // controlla che il magazzino nuovo sia diverso da quello vecchio e che siano un magazzino e uno store validi
  return (mv.store < in.Stores() && mv.new_w < in.Warehouses() && mv.old_w != mv.new_w);
} 

void WL_ChangeNeighborhoodExplorer::MakeMove(WL_Output& st, const WL_Change& mv) const
{
  // assegna il nuovo magazzino allo store
  st.Assign(mv.store, mv.new_w);
}  

void WL_ChangeNeighborhoodExplorer::FirstMove(const WL_Output& st, WL_Change& mv) const
{
  mv.store = 0; // inizia col primo store
  mv.old_w = st.Assignment(mv.store);
    
  // trova la prima warehouse disponibile
  mv.new_w = 0;
  
  // skippa il magazzino corrente se uguale al nuovo
  while (mv.new_w == mv.old_w){
    mv.new_w++;
  }
  
  // se abbiamo superato il numero di magazzini, passiamo al prossimo store
  if (mv.new_w >= in.Warehouses()){
    NextMove(st, mv);
  }
}

bool WL_ChangeNeighborhoodExplorer::NextMove(const WL_Output& st, WL_Change& mv) const
{
  do
    if (!AnyNextMove(st,mv))
      return false;
  while (!FeasibleMove(st,mv));
  return true;
}

bool WL_ChangeNeighborhoodExplorer::AnyNextMove(const WL_Output& st, WL_Change& mv) const
{
  mv.new_w++;
  
  // skippa il magazzino corrente se uguale al nuovo
  if (mv.new_w == mv.old_w)
    mv.new_w++;
  
  if(mv.new_w >= in.Warehouses())
  {
    mv.store++;

    if(mv.store >= in.Stores())
      return false;

    mv.old_w = st.Assignment(mv.store);
    mv.new_w = 0;

    if(mv.old_w == mv.new_w) // skippiamo il magazzino corrente se uguale al nuovo
      mv.new_w++; 
    
    if(mv.new_w >= in.Warehouses())
      return false; // assicura che non si superi il numero di magazzini
  }
  return true;
}

int WL_ChangeDeltaSupply::ComputeDeltaCost(const WL_Output& st, const WL_Change& mv) const
{
  int cost = 0;

  if (mv.old_w != -1) // se lo store era assegnato a un magazzino rimuvi il costo della fornitura dal vecchio magazzino
    cost -= in.AmountOfGoods(mv.store) * in.SupplyCost(mv.store, mv.old_w);
  
  cost += in.AmountOfGoods(mv.store) * in.SupplyCost(mv.store, mv.new_w);
  // aggiungi il costo della fornitura al nuovo magazzino

  return cost;
}

int WL_ChangeDeltaOpening::ComputeDeltaCost(const WL_Output& st, const WL_Change& mv) const
{
  int cost = 0;
  
  if (mv.old_w != -1 && st.Load(mv.old_w) == in.AmountOfGoods(mv.store)) 
    // se lo store era l'unico assegnato al vecchio magazzino
    cost -= in.FixedCost(mv.old_w); // rimuovi il costo fisso del vecchio magazzino
  
  if (st.Load(mv.new_w) == 0) 
    // se il nuovo magazzino era chiuso
    cost += in.FixedCost(mv.new_w); // aggiungi il costo fisso del nuovo magazzino

  return cost;
}

int WL_ChangeDeltaCapacity::ComputeDeltaCost(const WL_Output& st, const WL_Change& mv) const
{
  int cost = 0;

  // Gestione del vecchio magazzino
  if (mv.old_w != -1) 
  {
    // Calcola la violazione prima della mossa per il vecchio magazzino
    if (st.Load(mv.old_w) > in.Capacity(mv.old_w))
      cost -= st.Load(mv.old_w) - in.Capacity(mv.old_w);
    
    // Calcola la violazione dopo la mossa per il vecchio magazzino
    unsigned new_load_old = st.Load(mv.old_w) - in.AmountOfGoods(mv.store);
    if (new_load_old > in.Capacity(mv.old_w))
      cost += new_load_old - in.Capacity(mv.old_w);
  }

  // Gestione del nuovo magazzino
  // Calcola la violazione prima della mossa per il nuovo magazzino
  if (st.Load(mv.new_w) > in.Capacity(mv.new_w))
    cost -= st.Load(mv.new_w) - in.Capacity(mv.new_w);
  
  // Calcola la violazione dopo la mossa per il nuovo magazzino
  unsigned new_load_new = st.Load(mv.new_w) + in.AmountOfGoods(mv.store);
  if (new_load_new > in.Capacity(mv.new_w))
    cost += new_load_new - in.Capacity(mv.new_w);

  return cost;
}