#include "RA_Helpers.hh"

using namespace EasyLocal::Debug;

int main(int argc, const char* argv[])
{
  ParameterBox main_parameters("main", "Main Program options");
  Parameter<string> instance("instance", "Input instance", main_parameters); 
  Parameter<int> seed("seed", "Random seed", main_parameters);
  Parameter<string> method("method", "Solution method (empty for tester)", main_parameters);   
  Parameter<string> init_state("init_state", "Initial state (to be read from file)", main_parameters);
  Parameter<string> output_file("output_file", "Write the output to a file (filename required)", main_parameters);
 
  // 3rd parameter: false = do not check unregistered parameters
  // 4th parameter: true = silent
  CommandLineParameters::Parse(argc, argv, false, true);  

  if (!instance.IsSet())
    {
      cout << "Error: --main::instance filename option must always be set" << endl;
      return 1;
    }
  WL_Input in(instance);

  if (seed.IsSet())
    Random::SetSeed(seed);
  
  // cost components: second parameter is the cost, third is the type (true -> hard, false -> soft)
  WL_Supply cc1(in, 1, false);
  WL_Opening cc2(in, 1, false);
  WL_Capacity cc3(in, 1, true);
 
  WL_ChangeDeltaSupply dcc1(in, cc1);
  WL_ChangeDeltaOpening dcc2(in, cc2);
  WL_ChangeDeltaCapacity dcc3(in, cc3);

  // helpers
  WL_SolutionManager WL_sm(in);
  WL_ChangeNeighborhoodExplorer WL_nhe(in, WL_sm);
  
  // All cost components must be added to the state manager
  WL_sm.AddCostComponent(cc1);
  WL_sm.AddCostComponent(cc2);
  WL_sm.AddCostComponent(cc3);
  
  // All delta cost components must be added to the neighborhood explorer
  WL_nhe.AddDeltaCostComponent(dcc1);
  WL_nhe.AddDeltaCostComponent(dcc2);
  WL_nhe.AddDeltaCostComponent(dcc3);
  
  // runners
  HillClimbing<WL_Input, WL_Output, WL_Change> WL_hc(in, WL_sm, WL_nhe, "HC");
  SteepestDescent<WL_Input, WL_Output, WL_Change> WL_sd(in, WL_sm, WL_nhe, "SD");
  SimulatedAnnealing<WL_Input, WL_Output, WL_Change> WL_sa(in, WL_sm, WL_nhe, "SA");

  // tester
  Tester<WL_Input, WL_Output> tester(in, WL_sm);
  MoveTester<WL_Input, WL_Output, WL_Change> swap_move_test(in, WL_sm, WL_nhe, "WL_Change move", tester); 

  SimpleLocalSearch<WL_Input, WL_Output> WL_solver(in, WL_sm, "WL solver");
  if (!CommandLineParameters::Parse(argc, argv, true, false))
    return 1;

  if (!method.IsSet())
    { // if no search method is set -> enter the tester
      if (init_state.IsSet())
	    tester.RunMainMenu(init_state);
      else
	    tester.RunMainMenu();
    }
  else
    {
      if (method == "SA")
        {
          WL_solver.SetRunner(WL_sa);
        }
      else if (method == "HC")
        {
          WL_solver.SetRunner(WL_hc);
        }
      else if (method == "SD")
        {
          WL_solver.SetRunner(WL_sd);
        }
	  else
	  {
		  cerr << "Unknown method " << static_cast<string>(method) << endl;
		  exit(1);
	  }
      SolverResult<WL_Input, WL_Output> result = WL_solver.Solve();
      WL_Output out = result.output;
      if (output_file.IsSet())
        { // write the output on the file passed in the command line
          ofstream os(static_cast<string>(output_file));
          os << out << endl;
          os << "Cost: " << result.cost.total << endl;
	      os << "Time: " << result.running_time << "s " << endl;
          os.close();
        }
      else
        { // write the solution in the standard output
          cout << out << endl;
          cout << "Cost: " << result.cost.total << endl;
		  cout << "Time: " << result.running_time << "s " << endl;					
        }
   }
  return 0;
}
