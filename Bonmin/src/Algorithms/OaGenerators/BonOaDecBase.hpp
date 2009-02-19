// (C) Copyright International Business Machines (IBM) 2006
// All Rights Reserved.
// This code is published under the Common Public License.
//
// Authors :
// P. Bonami, International Business Machines
//
// Date :  12/07/2006
//#define OA_DEBUG
#ifndef BonOaDecBase_HPP
#define BonOaDecBase_HPP
#include "BonSubMipSolver.hpp"
#include "CglCutGenerator.hpp"
#include "BonBabSetupBase.hpp"
#include "BonOAMessages.hpp"
#include "CbcModel.hpp"

#include "CbcStrategy.hpp"

#include "CoinTime.hpp"
#include "OsiAuxInfo.hpp"
#include "OsiBranchingObject.hpp"
#include <iostream>
#include "BonBabInfos.hpp"
//#define OA_DEBUG
namespace Bonmin
{
  /** Base class for OA algorithms.*/
  class OaDecompositionBase : public CglCutGenerator
  {
  public:


    /** Small class to manipulatee various things in an OsiSolverInterface and restore them.
        The OsiSolverInterface manipulated may already exist or may be cloned from another one.*/
    class solverManip
    {
    public:
      /** Constructor. */
      solverManip(OsiSolverInterface *si , bool saveNumRows=true,
          bool saveBasis=true, bool saveBounds=false,
          bool saveCutoff = false, bool resolve=true);

      /** Constructor which clone an other interface. */
      solverManip(const OsiSolverInterface & si);
      /** Destructor. */
      ~solverManip();
      /** Restore solver. */
      void restore();

#if 0
      /** Clone the state of another solver (bounds, cutoff, basis).*/
      void cloneOther(const OsiSolverInterface &si);
#endif

      
      /** Get pointer to solver interface. */
      OsiSolverInterface * si()
      {
        return si_;
      }

      /** Set objects.*/
      void setObjects(OsiObject ** objects, int nObjects)
      {
        objects_ = objects;
        nObjects_ = nObjects;
      }

    private:
      /** Interface saved. */
      OsiSolverInterface * si_;
      /** Initial number of rows (-1 if don't save). */
      int initialNumberRows_;

      /** Initial lower bounds. */
      double * colLower_;

      /** Initial Upper bounds.*/
      double * colUpper_;

      /** Inital basis. */
      CoinWarmStart * warm_;

      /** Initial cutoff. */
      double cutoff_;

      /** delete si_ ? */
      bool deleteSolver_;

      /// Some objects the feasiblitiy of which to verify.
      OsiObject * * objects_;
      /// Number of objects.*/
      int nObjects_;
      /** \name Cached info from solver interface.*/
      /** @{ */
      /** Number of columns. */
      int numcols_;
      /** Number of rows. */
      int numrows_;
      /** Lower bounds on variables.*/
      const double * siColLower_;
      /** Upper bounds on variables. */
      const double * siColUpper_;

      void getCached();
      /** @} */
    };
    /// Old usefull constructor
    OaDecompositionBase(OsiTMINLPInterface * nlp = NULL
#if 0
  ,
        OsiSolverInterface * si = NULL,
        CbcStrategy * strategy = NULL,
        double cbcCutoffIncrement_=1e-07,
        double cbcIntegerTolerance = 1e-05,
        bool leaveSiUnchanged = 0
#endif
                       );
    /// New usefull constructor
    OaDecompositionBase(BabSetupBase &b, bool leaveSiUnchanged,
        bool reassignLpsolver);

    /// Copy constructor
    OaDecompositionBase(const OaDecompositionBase & copy);


    /// Destructor
    virtual ~OaDecompositionBase();

    /** Standard cut generation methods. */
    virtual void generateCuts(const OsiSolverInterface &si,  OsiCuts & cs,
        const CglTreeInfo info = CglTreeInfo()) const;

    /// Assign an OsiTMINLPInterface
    void assignNlpInterface(OsiTMINLPInterface * nlp)
    {
      nlp_ = nlp;
    }

    /// Assign an OsiTMINLPInterface
    void assignLpInterface(OsiSolverInterface * si)
    {
      lp_ = si;
    }

    bool reassignLpsolver()
    {
      return reassignLpsolver_;
    }
    /** Set objects.*/
    void setObjects(OsiObject ** objects, int nObjects)
    {
      objects_ = objects;
      nObjects_ = nObjects;
    }
    /// Set whether to leave the solverinterface unchanged
    inline void setLeaveSiUnchanged(bool yesno)
    {
      leaveSiUnchanged_ = yesno;
    }

    /** Parameters for algorithm. */
    struct Parameters
    {
      /// Add cuts as global
      bool global_;
      /// Add only violated OA inequalities
      bool addOnlyViolated_;
      /// cutoff min increase (has to be intialized trhough Cbc)
      double cbcCutoffIncrement_;
      /// integer tolerance (has to be the same as Cbc's)
      double cbcIntegerTolerance_;
      ///Max number of nodes for local search
      int localSearchNodeLimit_;
      ///Max number of local searches per node
      int maxLocalSearchPerNode_;
      ///Total max number of local searches
      int maxLocalSearch_;
      /// maximum time for local searches
      double maxLocalSearchTime_;
      /** sub milp log level.*/
      int subMilpLogLevel_;
      /** maximum number of solutions*/
      int maxSols_;
      /** Frequency of log. */
      double logFrequency_;

      /** Constructor with default values */
      Parameters();

      /** Copy constructor */
      Parameters(const Parameters & other);

      /** Destructor */
      ~Parameters()
      {
        if (!strategy_) delete strategy_;
      }

      /** Strategy to apply when using Cbc as MILP sub-solver.*/
      void setStrategy(const CbcStrategy & strategy)
      {
        if (strategy_) delete strategy_;
        strategy_ = strategy.clone();
      }

      const CbcStrategy * strategy() const
      {
        return strategy_;
      }
private:
      /** Strategy to apply when using Cbc as MILP sub-solver.*/
      CbcStrategy * strategy_;

    };

    Parameters& parameter()
    {
      return parameters_;
    }

    const Parameters& parameter()const
    {
      return parameters_;
    }

    void setLogLevel(int level)
    {
      handler_->setLogLevel(level);
    }

    void passInMessageHandler(CoinMessageHandler * handler);
  protected:
    /// \name Protected helper functions
    /**@{ */

    /** Solve the nlp and do output.
        \return true if feasible*/
    bool post_nlp_solve(BabInfo * babInfo, double cutoff) const;
    /** @} */

    /// virtual method which performs the OA algorithm by modifying lp and nlp.
    virtual double performOa(OsiCuts &cs, solverManip &lpManip,
        SubMipSolver * &subMip, BabInfo * babInfo, double &) const = 0;
    /// virutal method to decide if local search is performed
    virtual bool doLocalSearch(BabInfo * babInfo) const = 0;

    /// \name Protected members
    /** @{ */
    /// Pointer to nlp interface
    mutable OsiTMINLPInterface * nlp_;
    ///Number of nlp solved done
    mutable int nSolve_;
    /// A linear solver
    mutable OsiSolverInterface * lp_;
    /// Some objects the feasiblitiy of which to verify.
    OsiObject * * objects_;
    /// Number of objects.*/
    int nObjects_;
    ///number of local searches performed
    mutable int nLocalSearch_;
    /** messages handler. */
    CoinMessageHandler * handler_;
    /** Messages for OA */
    CoinMessages messages_;
    /** Wether or not we should remove cuts at the end of the procedure */
    bool leaveSiUnchanged_;
    /** Do we need to reassign the lp solver with Cbc.*/
    bool reassignLpsolver_;
    /** time of construction*/
    double timeBegin_;
    /** number of solutions found by OA_decomposition.*/
    mutable int numSols_;

    /** Parameters.*/
    Parameters parameters_;

    /** @} */

#ifdef OA_DEBUG
    class OaDebug
    {
      public:
      bool checkInteger(const OsiSolverInterface&nlp, std::ostream & os) const;

      void printEndOfProcedureDebugMessage(const OsiCuts &cs,
          bool foundSolution,
          double solValue,
          double milpBound,
          bool isInteger,
          bool feasible,
          std::ostream & os) const;
    };

    /** debug object. */
    OaDebug debug_;

#endif
  };
}
#endif

